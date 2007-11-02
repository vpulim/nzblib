#include <unistd.h>
#include <stdio.h>

#import <nzb_fetch.h>

int main(int argc, char **argv)
{
    nzb_fetch *fetcher;
    nzb_file *nzb_file;
    
    // Initialize
    fetcher = nzb_fetch_init();
    
    // TODO: Hook a function to be called when a file is complete
    // nzb_fetch_bind_file_complete(myfunction)
    
    // TODO: Hook a function to be called when a nzb file is complete
    // nzb_fetch_bind_nzb_complete(myfunction)


    // Add servers. 119 = port, 3 = number of connection threads
    // and the last argument is the priority, the lower the better.
    // Articles which can't be found on my.newsserver.nl are downloaded from
    // my.fillserver.nl
    nzb_fetch_add_server(fetcher, "my.newsserver.nl", 119,
                         "username", "password", 3, 0);

    nzb_fetch_add_server(fetcher, "my.fillserver.nl", 119,
                         "username", "password", 3, 999); 
    
    // Connect to the server(s)
    nzb_fetch_connect(fetcher);

    // Pass the filename of the nzb file
    nzb_file = nzb_fetch_parse(sys.argv[1]);

    // Set the directory where the file is stored
    nzb_fetch_storage_path(nzb_file, ".");
    
    // Set the working directory
    nzb_fetch_temporary_path(nzb_file, "./chunks/");
    
    // Start downloading
    nzb_fetch_download(fetcher, nzb_file);

    // Since all work happens in other threads we need to wait here.
    while(1)
    {
        sleep(1);
        
        // TODO: Here should be a function to request the status
        
        printf(".\n");
    }
    
    
    // TODO: Disconnect
    // nzb_fetch_disconnect(fetcher);
    
    // TODO: Clean up
    // nzb_fetch_destroy(fetcher);
    
    
    return 0;
}