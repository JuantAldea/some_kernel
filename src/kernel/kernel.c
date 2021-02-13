#include "kernel.h"
#include "../termio/termio.h"
#include "../idt/idt.h"
#include "../memory/memory.h"
#include "../memory/kheap.h"
#include "../io/io.h"
#include "../memory/paging.h"
#include "../disk/disk.h"
#include "../disk/disk_stream.h"
#include "../fs/path_parser.h"
#include "../string/string.h"

void kernel_splash()
{
    print("\n\n\n\n\n");
    terminal_put_str("                  _                                _    ___  __\n", 2);
    terminal_put_str("                 /_\\ __   __ ___    ___  __ _   __| |  /___\\/ _\\\n", 2);
    terminal_put_str("                //_\\\\\\ \\ / // _ \\  / __|/ _` | / _` | //  //\\ \\\n", 2);
    terminal_put_str("               /  _  \\\\ V /| (_) || (__| (_| || (_| |/ \\_// _\\ \\\n", 2);
    terminal_put_str("               \\_/ \\_/ \\_/  \\___/  \\___|\\__,_| \\__,_|\\___/  \\__/\n", 2);
}

static struct page_directory_handle *kernel_chunk = NULL;

void print_path(struct path_root *path)
{
    struct path_part *part = path->first;
    print("Drive: ");
    terminal_put_char(digit_to_char(path->drive_number), 15);
    terminal_put_char('\n', 15);
    while (part) {
        print(part->part);
        print("\n");
        part = part->next;
    }
}

void kernel_main(void)
{
    terminal_init();
    kernel_splash();
    print("Starting...\n");

    kheap_init();

    idt_init();

    disk_discover_and_init();


    kernel_chunk = page_directory_init_4gb(PAGING_WRITABLE_PAGE
                    | PAGING_PRESENT
                    | PAGING_ACCESS_FROM_ALL);
    paging_switch_directory(kernel_chunk);

    char *page_ptr = kzalloc(4096);

    // map the region pointed by page_ptr to virtual addr 0x1000

    paging_map_v_addr(kernel_chunk, (void*) 0x1000, page_ptr,
                            PAGING_ACCESS_FROM_ALL
                            | PAGING_PRESENT
                            | PAGING_WRITABLE_PAGE);

    // also to addr 0x2000
    paging_map_v_addr(kernel_chunk, (void*) 0x2000, page_ptr,
                            PAGING_ACCESS_FROM_ALL
                            | PAGING_PRESENT
                            | PAGING_WRITABLE_PAGE);

    char buf[512];

    struct disk* master_disk = disk_get(0);

    enable_interrupts();

    disk_read_block(master_disk, 0, 1, &buf);


    char path[] = "1:/Compilers/Principles/Techniques/and/Tools";
    print(path);
    print("\n");
    struct path_root *root = pathparser_path_parse(path);
    if (root) {
        print_path (root);
        print("\n");
    } else {
        print("NULL\n");
    }

    struct disk_stream *stream = diskstream_open(0);
    unsigned char *buffer = kmalloc(512);
    diskstream_seek(stream, 0x1FE);
    diskstream_read(stream, buffer, 10);

    print("Done for now\n");
trap:
    goto trap;
}
