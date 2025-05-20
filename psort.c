/*  
    psort.c  – Parallel, stable sorter with a dynamic work queue
    Dylan Prosser – CSC 139 (Section 06)
    Linux distribution and version - Ubuntu 24.04.1 LTS
    Kernel release - 4.4.0-19041 - Microsoft
    RAM (GB) - 15.9 
    OS - Windows 10 Home,    Version: 22H2,   OS build:	19045.5737
    Compiler - gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0 

    lscpu output:

    Architecture:            x86_64
  CPU op-mode(s):        32-bit, 64-bit
  Address sizes:         36 bits physical, 48 bits virtual
  Byte Order:            Little Endian
CPU(s):                  12
  On-line CPU(s) list:   0-11
Vendor ID:               AuthenticAMD
  Model name:            AMD Ryzen 5 5600X 6-Core Processor
    CPU family:          25
    Model:               33
    Thread(s) per core:  2
    Core(s) per socket:  6
    Socket(s):           1
    Stepping:            2
    CPU(s) scaling MHz:  100%
    CPU max MHz:         3701.0000
    CPU min MHz:         0.0000
    BogoMIPS:            7402.00
    Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse ss
                         e2 ht syscall nx mmxext fxsr_opt pdpe1gb rdtscp lm pni pclmulqdq monitor ssse3 fma cx16 sse4_1
                         sse4_2 movbe popcnt aes xsave osxsave avx f16c rdrand lahf_lm cmp_legacy svm extapic cr8_legacy
                          abm sse4a misalignsse 3dnowprefetch osvw ibs skinit wdt tce topoext perfctr_core perfctr_nb bp
                         ext perfctr_llc mwaitx fsgsbase bmi1 avx2 smep bmi2 erms invpcid cqm rdt_a rdseed adx smap clfl
                         ushopt clwb sha_ni umip pku vaes vpclmulqdq rdpid
Virtualization features:
  Virtualization:        AMD-V
  Hypervisor vendor:     Windows Subsystem for Linux
  Virtualization type:   container
 */

 #define _POSIX_C_SOURCE 200809L
 #include <errno.h>
 #include <fcntl.h>
 #include <inttypes.h>
 #include <pthread.h>
 #include <stdbool.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/mman.h>
 #include <sys/stat.h>
 #include <sys/time.h>
 #include <sys/types.h>
 #include <time.h>
 #include <unistd.h>
 
 #define RECORD_SIZE     128u
 #define KEY_SIZE        4u
 #define CHUNK_RECORDS   8192u  
 #define MAX_THREADS     64u
 
 static double now_sec(void)
 {
     struct timespec ts;
     clock_gettime(CLOCK_MONOTONIC, &ts);
     return ts.tv_sec + ts.tv_nsec / 1e9;
 }
 
 typedef struct {
     uint8_t bytes[RECORD_SIZE];
 } record_t;
 
 static inline uint32_t rec_key(const record_t *r)
 {
     uint32_t k;
     memcpy(&k, r->bytes, KEY_SIZE); 
     return k;
 }
 
 static void merge(record_t *dst,
                   record_t *src,
                   size_t left, size_t mid, size_t right)
 {
     size_t i = left, j = mid, k = left;
     while (i < mid && j < right) {
         if (rec_key(&src[i]) <= rec_key(&src[j])) 
             dst[k++] = src[i++];
         else
             dst[k++] = src[j++];
     }
     while (i < mid)  dst[k++] = src[i++];
     while (j < right) dst[k++] = src[j++];
 }
 
 static void mergesort_rec(record_t *buf,
                           record_t *tmp,
                           size_t left,
                           size_t right)
 {
     if (right - left <= 32) {     
         for (size_t i = left + 1; i < right; ++i) {
             record_t key = buf[i];
             size_t j = i;
             while (j > left && rec_key(&buf[j - 1]) > rec_key(&key)) {
                 buf[j] = buf[j - 1];
                 --j;
             }
             buf[j] = key;
         }
         return;
     }
 
     size_t mid = left + (right - left) / 2;
     mergesort_rec(buf, tmp, left, mid);
     mergesort_rec(buf, tmp, mid, right);
 
     memcpy(tmp + left, buf + left, (right - left) * sizeof(record_t));
     merge(buf, tmp, left, mid, right);
 }
 
 static void mergesort(record_t *buf, size_t n)
 {
     record_t *tmp = malloc(n * sizeof(record_t));
     if (!tmp) { perror("malloc"); exit(EXIT_FAILURE); }
     mergesort_rec(buf, tmp, 0, n);
     free(tmp);
 }
 
 typedef struct {
     size_t start_idx; 
     size_t count; 
 } task_t;
 
 typedef struct {
     task_t *arr;
     size_t  cap, head, tail, size;
     bool    done;
 
     pthread_mutex_t mtx;
     pthread_cond_t  cv;
 } queue_t;
 
 static void q_init(queue_t *q, size_t cap)
 {
     q->arr  = malloc(cap * sizeof(task_t));
     q->cap  = cap;
     q->head = q->tail = q->size = 0;
     q->done = false;
     pthread_mutex_init(&q->mtx, NULL);
     pthread_cond_init(&q->cv,  NULL);
 }
 
 static void q_destroy(queue_t *q)
 {
     free(q->arr);
     pthread_mutex_destroy(&q->mtx);
     pthread_cond_destroy(&q->cv);
 }
 
 static void q_push(queue_t *q, task_t t)
 {
     q->arr[q->tail] = t;
     q->tail = (q->tail + 1) % q->cap;
     q->size++;
 }
 
 static bool q_pop(queue_t *q, task_t *out)
 {
     if (q->size == 0) return false;
     *out = q->arr[q->head];
     q->head = (q->head + 1) % q->cap;
     q->size--;
     return true;
 }
 
 typedef struct {
     queue_t    *q;
     record_t   *records;
 } worker_arg_t;
 
 static void *worker_fn(void *argp)
 {
     worker_arg_t *wa = argp;
     task_t t;
 
     for (;;) {
         pthread_mutex_lock(&wa->q->mtx);
         while (wa->q->size == 0 && !wa->q->done)
             pthread_cond_wait(&wa->q->cv, &wa->q->mtx);
 
         if (!q_pop(wa->q, &t)) {
             if (wa->q->done) {
                 pthread_mutex_unlock(&wa->q->mtx);
                 break; 
             }
             pthread_mutex_unlock(&wa->q->mtx);
             continue;
         }
         pthread_mutex_unlock(&wa->q->mtx);
 
         mergesort(wa->records + t.start_idx, t.count);
     }
     return NULL;
 }

 static void pairwise_merge(record_t *buf, record_t *tmp,
                            size_t start, size_t mid, size_t end)
 {
     memcpy(tmp + start, buf + start, (end - start) * sizeof(record_t));
     merge(buf, tmp, start, mid, end);
 }
 
 static void merge_all_chunks(record_t *buf,
                              size_t total_records,
                              size_t chunk_size)
 {
     record_t *tmp = malloc(total_records * sizeof(record_t));
     if (!tmp) { perror("malloc"); exit(EXIT_FAILURE); }
 
     size_t current_chunk = chunk_size;
     while (current_chunk < total_records) {
         for (size_t start = 0; start < total_records; start += 2 * current_chunk) {
             size_t mid  = start + current_chunk;
             size_t end  = start + 2 * current_chunk;
             if (mid >= total_records) break;
             if (end > total_records) end = total_records;
             pairwise_merge(buf, tmp, start, mid, end);
         }
         current_chunk *= 2;
     }
     free(tmp);
 }

 static record_t *slurp_file(const char *path, size_t *out_records)
 {
     int fd = open(path, O_RDONLY);
     if (fd < 0) { perror("open input"); exit(EXIT_FAILURE); }
 
     struct stat st;
     if (fstat(fd, &st) < 0) { perror("fstat"); exit(EXIT_FAILURE); }
     if (st.st_size % RECORD_SIZE != 0) {
         fprintf(stderr, "Input size not a multiple of %u bytes\n", RECORD_SIZE);
         exit(EXIT_FAILURE);
     }
     size_t nrecords = st.st_size / RECORD_SIZE;
     record_t *buf = malloc(st.st_size);
     if (!buf) { perror("malloc"); exit(EXIT_FAILURE); }
 
     ssize_t nread = read(fd, buf, st.st_size);
     if (nread != st.st_size) { perror("read"); exit(EXIT_FAILURE); }
     close(fd);
 
     *out_records = nrecords;
     return buf;
 }
 
 static void flush_and_close(int fd)
 {
     if (fsync(fd) < 0) { perror("fsync"); }
     close(fd);
 }
 
 static void dump_file(const char *path, const record_t *buf, size_t nrecords)
 {
     int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
     if (fd < 0) { perror("open output"); exit(EXIT_FAILURE); }
 
     size_t nbytes = nrecords * RECORD_SIZE;
     ssize_t nw = write(fd, buf, nbytes);
     if (nw != (ssize_t)nbytes) { perror("write"); exit(EXIT_FAILURE); }
 
     flush_and_close(fd);
 }

 int main(int argc, char **argv)
 {
     if (argc != 3) {
         fprintf(stderr, "Usage: %s input.bin output.bin\n", argv[0]);
         return EXIT_FAILURE;
     }

     double t0 = now_sec();
     size_t nrecords = 0;
     record_t *records = slurp_file(argv[1], &nrecords);
     double t_read = now_sec() - t0;

     size_t chunk = CHUNK_RECORDS;
     if (chunk > nrecords) chunk = nrecords;
 
     size_t ntasks = (nrecords + chunk - 1) / chunk;
     queue_t q;
     q_init(&q, ntasks);
 
     for (size_t i = 0; i < ntasks; ++i) {
         size_t start = i * chunk;
         size_t cnt   = (start + chunk <= nrecords) ? chunk : (nrecords - start);
         q_push(&q, (task_t){ .start_idx = start, .count = cnt });
     }
 
     long ncpu = sysconf(_SC_NPROCESSORS_ONLN);
     size_t nthreads = ntasks < (size_t)ncpu ? ntasks : (size_t)ncpu;
     if (nthreads > MAX_THREADS) nthreads = MAX_THREADS;
     if (nthreads == 0) nthreads = 1;
 
     pthread_t tids[MAX_THREADS];
     worker_arg_t warg = { .q = &q, .records = records };
 
     double t1 = now_sec();
     for (size_t i = 0; i < nthreads; ++i)
         pthread_create(&tids[i], NULL, worker_fn, &warg);
 
     pthread_mutex_lock(&q.mtx);
     q.done = true;
     pthread_cond_broadcast(&q.cv);
     pthread_mutex_unlock(&q.mtx);
 
     for (size_t i = 0; i < nthreads; ++i)
         pthread_join(tids[i], NULL);
     double t_sort = now_sec() - t1;

     double t2 = now_sec();
     merge_all_chunks(records, nrecords, chunk);
     dump_file(argv[2], records, nrecords);
     double t_merge = now_sec() - t2;
 
     fprintf(stderr,
             "Read:  %.3fs | Sort: %.3fs | Merge+Write: %.3fs | Total: %.3fs\n",
             t_read, t_sort, t_merge, t_read + t_sort + t_merge);
 
     q_destroy(&q);
     free(records);
     return EXIT_SUCCESS;
 }
 