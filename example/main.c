#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#import <nzb_fetch.h>

int main(int argc, char **argv)
{
    nzb_fetch *fetcher;
    nzb_file *nzb_file;
    nzb_file_info **files;
    
    int num_files;
    int i;
    
    if (argc < 4)
    {
        printf("Usage: \n");
        printf("%s <nzb_file> <server> <username> <password> <connections>\n", argv[0]);
        exit(1);
    } 
    
    fetcher = nzb_fetch_init();

    nzb_fetch_add_server(fetcher, argv[2], 119,
                        argv[3], argv[4], 2, 0);
    
    
    nzb_fetch_connect(fetcher);

    nzb_file = nzb_fetch_parse( argv[1]);
    nzb_fetch_storage_path(nzb_file, strdup("tmp/complete/"));
    nzb_fetch_temporary_path(nzb_file, strdup("tmp/chunks/"));

        
    num_files = nzb_fetch_list_files(nzb_file, &files);

    
    for (i = 0; i < num_files; i++)
    {
        nzb_fetch_download(fetcher, files[i]);
        if (strstr(files[i]->filename, ".rar") != NULL)
        {
            printf("nzb_fetch_download(%s)\n", files[i]->filename);
        }
    }
    
    while(1)
    {
        sleep(1);
        printf(".\n");
    }

    //
    
    return 0;
}
