#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <signal.h>

#include "picos.h"
#include "picos_private.h"

struct {
    bool            initialised;
    int             verbosity;

    int             curr_region_id;
    int             chkpt_counter;
    ullong          tot_size;
    picos_region          *region_list_head;
    picos_region          *region_list_end;

    bool            do_ddump;
    int             ddump_interval;

    int             ddump_fd;
    byte            *ddump_file_ptr, *ddump_curr_ptr;
    ullong          ddump_file_bytes;
    char            *ddump_filename;

    bool            text_registered;
    picos_page      *page_list_head;
    picos_page      *page_list_end;
} picos = {0};

void picos_init()
{
    dbg("picos_init()\n");
    if (picos.initialised)
        die("Must not call picos_init() twice\n");

    char *env;
    if ((env = getenv("PICOS_DEBUG")))
    {
        picos.verbosity = ez_strtoul(env);
        if (picos.verbosity == -1)
            elog("Warning: PICOS_DEBUG could not be parsed, string: \"%s\"\n", env);
    }

    signal(SIGINT, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGBUS, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGQUIT, signal_handler);

    picos.initialised   = true;
}

int vma_iter_callback(void *page_list_end_ptr, uintptr_t start, uintptr_t end, unsigned int flags)
{
    picos_page *page_list_end  = *((picos_page **) page_list_end_ptr);
    page_list_end->start = (byte *) start;
    page_list_end->end   = (byte *) end;
    page_list_end->flags = flags;
    page_list_end->next  = (*((picos_page **) page_list_end_ptr) = calloc(1, sizeof(picos_page)));

    return 0;
}

picos_page *picos_get_xpages()
{
    dbg("picos_register_text()\n");
    if (picos.text_registered)
        die("must never call picos_register_text() twice\n");
    picos.text_registered = true;
    picos.page_list_head = (picos.page_list_end = calloc(1, sizeof(picos_page)));

    vma_iterate(vma_iter_callback, (void *) & (picos.page_list_end));

    free(picos.page_list_end->next);
    picos.page_list_end->next = NULL;

    return picos.page_list_head;
}

void __picos_register(bool is_ptr, void **ptr_or_double_ptr, size_t size)
{
    dbg("picos_register()\n");
    check_init("picos_register");

    picos_region *r               = malloc(sizeof(picos_region));
    r->id                   = picos.curr_region_id++;
    r->size                 = size;
    r->is_ptr               = is_ptr;
    r->ptr_to_origin        = (byte **) ptr_or_double_ptr;
    r->image                = calloc(1, size);
    r->next                 = NULL;

    picos.region_list_end   = picos.region_list_end == NULL ?
                              (picos.region_list_head = r) :
                              (picos.region_list_end->next = r);

    picos.tot_size          += size;
}

void picos_register_primitive(void *ptr_to_prim, size_t size)
{
    __picos_register(false, (void **) ptr_to_prim, size);
}

void picos_register_ptr(void **ptr_to_ptr_to_data, size_t size)
{
    __picos_register(true, ptr_to_ptr_to_data, size);
}

void picos_enable_disk_dump(const char *prefix, int every_n_chkpts)
{
    dbg("picos_enable_disk_dump()\n");
    check_init("picos_enable_disk_dump");

    picos.do_ddump          = true;
    picos.ddump_interval    = every_n_chkpts;

    format_ddump_filename(&(picos.ddump_filename), prefix, 0);
    dbg("ddump_filename set up as \"%s\"\n", picos.ddump_filename);

    //create checkpoint file with the right specs
    FILE *fp = fopen(picos.ddump_filename, "w");
    fseek(fp, picos.tot_size, SEEK_SET);

    //make a mark at the final (extra) byte of the file to create the sized file
    fputc('\0', fp);
    fclose(fp);

    //map checkpoint file into memory
    picos.ddump_fd  = open(picos.ddump_filename, O_RDWR, 0600);
    if (!picos.ddump_fd)
        die("Could not open chkpt file \"%s\" (%s)\n", picos.ddump_filename, strerror(errno));

    picos.ddump_file_ptr = mmap(0, picos.tot_size, PROT_WRITE, MAP_SHARED, picos.ddump_fd, 0);
    if (picos.ddump_file_ptr == MAP_FAILED)
        die("Could not map chkpt file into memory (%s)\n", strerror(errno));

    ddump_flush();
}

void picos_checkpoint_now()
{
    dbg("picos_checkpoint()\n");
    check_init("picos_checkpoint");

    double startTime = get_unix_ms();

    const bool do_ddump_now =
        picos.do_ddump &&
        (picos.chkpt_counter % picos.ddump_interval == 0);

    picos_region *ptr = picos.region_list_head;
    while (ptr)
    {
        byte *origin = ptr->is_ptr ? *(ptr->ptr_to_origin) : (byte *) (ptr->ptr_to_origin);

        dbg("Checkpointing region #%d, %p->%p %ld bytes\n",
            ptr->id, origin, ptr->image, ptr->size);

        memcpy(ptr->image, origin, ptr->size);

        if (do_ddump_now)
        {
            memcpy(picos.ddump_curr_ptr, ptr->image, ptr->size);
            picos.ddump_curr_ptr += ptr->size;
        }

        ptr = ptr->next;
    }

    picos.chkpt_counter++;

    if (do_ddump_now)
        ddump_flush();

    double timeDiff = get_unix_ms() - startTime;

    dbg("Checkpoint #%d done: %llu bytes in %.2f ms\n",
        picos.chkpt_counter, picos.tot_size, timeDiff / 1000);
}

void picos_warm_recover()
{
    dbg("picos_warm_recover()\n");
    check_init("picos_warm_recover");

    picos_region *ptr = picos.region_list_head;
    while (ptr)
    {
        byte *origin = ptr->is_ptr ? *(ptr->ptr_to_origin) : (byte *) (ptr->ptr_to_origin);

        dbg("Recovering region #%d, %p->%p %ld bytes\n",
            ptr->id, ptr->image, origin, ptr->size);

        memcpy(origin, ptr->image, ptr->size);

        ptr = ptr->next;
    }
}

void picos_cold_recover(const char* prefix, long from_pid)
{
    dbg("picos_cold_recover(%ld)\n", from_pid);
    check_init("picos_cold_recover");

    char *chkpt_filename;
    format_ddump_filename(&(chkpt_filename), prefix, from_pid);

    //map checkpoint file into memory
    int chkpt_fd = open(chkpt_filename, O_RDWR, 0600);
    if (!chkpt_fd)
        die("could not open checkpoint file \"%s\" (%s)\n", chkpt_filename, strerror(errno));

    byte *chkpt_ptr = mmap(0, picos.tot_size, PROT_WRITE, MAP_SHARED, chkpt_fd, 0);
    if (chkpt_ptr == MAP_FAILED)
        die("could not map checkpoint file into memory (%s)\n", strerror(errno));

    picos_region *ptr = picos.region_list_head;
    while (ptr)
    {
        byte *origin = ptr->is_ptr ? *(ptr->ptr_to_origin) : (byte *) (ptr->ptr_to_origin);

        dbg("Cold recovery from chkpt file %s; region #%d (%ld bytes)\n",
            chkpt_filename, ptr->id, ptr->size);

        memcpy(origin, chkpt_ptr, ptr->size);
        memcpy(ptr->image, chkpt_ptr, ptr->size);

        chkpt_ptr += ptr->size;
        ptr = ptr->next;
    }

    munmap(chkpt_ptr, picos.tot_size);
    close(chkpt_fd);
}

void picos_finalise()
{
    dbg("picos_client_finalise()\n");
    check_init("picos_client_finalise");

    free_resources();

    dbg("picos_finalise() complete\n");
}


/* Private functions */

void signal_handler(int sig)
{
    fprintf(stderr, "[picos fatal] signal_handler()\n");
    fprintf(stderr, "[picos fatal] Caught %s\n", strsignal(sig));

    free_resources();

    exit(sig);
}

void get_time_string(char **buffer_ptr, const char* fmt)
{
    struct tm *tm;
    time_t t;

    t = time(NULL);
    tm = localtime(&t);

    *(buffer_ptr) = (char *) malloc(64 * sizeof(char));

    strftime(*(buffer_ptr), 64, "%Y-%m-%d %H:%M:%S", tm);
}

double get_unix_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_usec + tv.tv_sec * 1e6) / 1000;
}

ulong ez_strtoul(const char *restrict str)
{
    char *invalid;
    ulong ul = strtoul(str, &invalid, 10);
    return (*invalid == '\0') ? ul : -1;
}

void format_ddump_filename(char **dst, const char *prefix, long cold_rcvr_pid)
{
    long pid = 0;
    char *suffix = "";

    IF_MPI(
    {
        int mpi_init = 0;
        MPI_Initialized(&mpi_init);

        if (mpi_init)
        {
            int my_rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

            long master_pid = 0;
            if (!my_rank)
                master_pid = (long) getpid();
            MPI_Bcast(&master_pid, 1, MPI_LONG, 0, MPI_COMM_WORLD);
            dbg("Rank %d, master_pid=%ld\n", my_rank, master_pid);

            pid = master_pid;

            asprintf(&suffix, "R%d.", my_rank);
        }
        else
        {
            die("MPI_Init() must be called before setting up disk dump\n");
        }
    }
    );

    IFN_MPI(
    {
        pid = getpid();
    }
    );

    asprintf(dst,
             "%s.%ld.%spicos",
             prefix,
             cold_rcvr_pid == 0 ? pid : cold_rcvr_pid,
             suffix);
}

void ddump_flush()
{
    //queue async write
    while (msync(picos.ddump_file_ptr, picos.tot_size, MS_ASYNC) == -1)
    {
        if (errno != EBUSY)
            die("daemon_disk_dump_flush(): msync() failed with error: %s\n", strerror(errno));

        /*
        If file is large enough to not finish
        writing between user checkpoints
        then sleeping 50ms is reasonable
        */
        log("disk wait\n");
        usleep(50 * 1000);
    }

    //reset ddump ptr
    picos.ddump_curr_ptr = picos.ddump_file_ptr;
}

void free_resources()
{
    dbg("free_resources()\n");

    //free regions
    picos_region *ptr = picos.region_list_head;
    if (ptr)
    {
        do
        {
            picos_region *tmp = ptr;
            ptr = ptr->next;

            free(tmp->image);
            free(tmp);
        } while (ptr);
    }

    if (picos.do_ddump)
    {
        //unmap and close checkpoint file
        if (picos.ddump_file_ptr)
            munmap(picos.ddump_file_ptr, picos.tot_size);

        if (picos.ddump_fd)
            close(picos.ddump_fd);
    }
}
