/*
 * Copyright (c) 2010-2012 Helsinki Institute for Information Technology
 * and University of Helsinki.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file
 *
 * @author Samu Varjonen
 *
 * This is not the fanciest of programming but serves its purpose
 * as the central error/warning handling process. This functionality
 * is meant to be reimplemented by the users. This just a simple 
 * example.
 */ 

/**
 * Needed for the getopt as I use -ansi
 */
#define _GNU_SOURCE

#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFLEN 1024 /**< Buffer length for receiving */

#define STAT_BUFFER_LEN 64 /**<Statistics buffer len */

#define WIN_BORDERS 2 /**< if window has borders */

#define BOTTOM 3 /**< where to start scrolling */

WINDOW *data_win; /**< window for data */
WINDOW *stat_win; /**< window for statistics */
WINDOW *err_win; /**< window for errors and warnings */

enum { NOTINUSE, DATA, STAT, ERROR, PLAIN }; /**< Window color pairs */

int data_height; /**< window size parameter */
int data_width; /**< window size parameter */
int data_startx; /**< window size parameter */
int data_starty; /**< window size parameter */
int stat_height; /**< window size parameter */
int stat_width; /**< window size parameter */
int stat_startx; /**< window size parameter */
int stat_starty; /**< window size parameter */
int err_height; /**< window size parameter */
int err_width; /**< window size parameter */
int err_startx; /**< window size parameter */
int err_starty; /**< window size parameter */

int nodes; /**< how many nodes are we expecting */
int port; /**< which port to listen */
int curses; /**< are we using curses 1 is to use */
int gnuplot; /**< if 1 write stats to file for gnuplot to read 
                  only with ncurses */

/**
 * Struct to hold all the statistics. 
 */
struct stats {
    int mepid; /**< ID of the seen MEP */
    int nmsg; /**< # of msgs received from this MEP */
    float min; /**< Minimum of reported delay */
    float mean; /**< Mean of reported delay */
    float max; /**< Maximum of reported delay */
    int errors; /**< num of seen errors */
    int warnings; /**< num of seen errors */
    int alarm; /**< alarm reported to */
    int rdi; /**< reported alarm from */
};

/**
 * @param app Application name
 */
void usage(char * app) 
{
        printf("usage: %s -p [port] -n [# of nodes] -c\n"
               "c is for ncurses mode\n", app);
	exit(-1);
}

/**
 * Creates a new ncurses window with specifix properties
 *
 * @param height Height of the window
 * @param width Width of the window 
 * @param starty Y coordinate of the upper left corner
 * @param startx X coordinate of the upper left corner
 * @param title What to draw as the title.
 *
 * @return pointer to the created window. 
 *
 * @note Ncurses coordinates, so lines and chars...
 */
WINDOW *create_newwin(int height, int width, 
                      int starty, int startx, 
                      char *title) 
{
    WINDOW *local_win;
	        
    local_win = newwin(height, width, starty, startx);    
    box(local_win, 0, 0);
    wmove(local_win, 0, 0);	    
    mvwaddstr(local_win, 0, 1, title);
    scrollok(local_win, TRUE);   
    wrefresh(local_win);

    return local_win; 
}

/**
 * @param win Window to be drawn again
 * @param title if there is a box what title should it has on the top
 */ 
void rwrefresh(WINDOW *win, char *title)
{
    box(win, 0, 0);
    mvwaddstr(win, 0, 1, title);
    wrefresh(win);
}

/**
 * As it sais this function initializes all the windows.
 */
void init_all_windows(void)
{
    initscr();
    cbreak();
    noecho();
    refresh();
    
    if (has_colors()) {
        start_color();
    }
    
    data_height = LINES - WIN_BORDERS * (nodes + WIN_BORDERS); 
    data_width  = COLS;
    data_startx = 0;        
    data_starty = 0;         
    
    stat_height = nodes + WIN_BORDERS;
    stat_width = COLS;
    stat_startx = 0;
    stat_starty = data_height;
    
    err_height = nodes + WIN_BORDERS; 
    err_width  = COLS;
    err_startx = 0;        
    err_starty = data_height + stat_height; 
        
    init_pair(DATA, COLOR_GREEN, COLOR_BLACK);
    init_pair(STAT, COLOR_BLUE, COLOR_BLACK);
    init_pair(ERROR, COLOR_RED, COLOR_BLACK);
    init_pair(PLAIN, COLOR_WHITE, COLOR_BLACK);
    
    data_win = create_newwin(data_height, data_width, 
                             data_starty, data_startx, 
                             "Data");
    
    stat_win = create_newwin(stat_height, stat_width, 
                             stat_starty, stat_startx, 
                             "Statistics");
    
    err_win = create_newwin(err_height, err_width, 
                            err_starty, err_startx, 
                            "Error");
    
    wbkgd(data_win, COLOR_PAIR(PLAIN));
    wbkgd(stat_win, COLOR_PAIR(PLAIN));
    wbkgd(err_win, COLOR_PAIR(PLAIN));
    
    wrefresh(data_win);
    wrefresh(stat_win);
    wrefresh(err_win); 
    
    refresh();
}

/**
 * refresh all windows
 */
void refresh_all_windows(void)
{
    rwrefresh(data_win, "Data");
    rwrefresh(stat_win, "Statistics");
    rwrefresh(err_win, "Errors");
}

/**
 * handle options
 * 
 * @param argc how many arguments
 * @param argv arguments
 */
void handle_options(int argc, char **argv)
{ 
    int c;
    
    curses = 0;
    nodes = 1;
    port = 8080; 
    gnuplot = 0;

    while ((c = getopt (argc, argv, "p:n:cg")) != -1) {
   	switch (c) {
        case 'c':
            curses = 1;
            break;
        case 'g':
            gnuplot = 1;
            break;
	case 'p':
            port = atoi(optarg);
            break;
	case 'n':
            nodes = atoi(optarg);
            break;
	default:
            usage(argv[0]);
            break;
	}
    }
}

/**
 * This is the main receiving loop
 *
 * @return Zero on succes, non-zero else.
 */
int main(int argc, char **argv)
{
    int current_line;
    int seen_nodes; 
    int s;
    int i;
    int found;
    int start;
    int howmany;
    
    char *where; 
    char *where2;

    char buf[BUFLEN];
    char time_buffer[25];
    char min_buffer[STAT_BUFFER_LEN];
    char mean_buffer[STAT_BUFFER_LEN];
    char max_buffer[STAT_BUFFER_LEN];

    time_t timer;
    socklen_t slen;

    FILE *f;

    struct tm* tm_info;
    struct sockaddr_in si_me, si_other; 

    slen = sizeof(si_other);
  
    if (argc < 2 || argc > 7) {
        usage(argv[0]);	
    }
 
    handle_options(argc, argv);
   
    found = seen_nodes = 0;
    
    struct stats mep_stats[nodes];
    
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        return -1;
    }
    
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me))==-1) {
        perror("bind");
        return -1;
    }    

    if (curses == 1) {
        /* CURSES MODE */
        memset(&mep_stats, 0, sizeof(mep_stats));
        
        init_all_windows();
        
        wmove(data_win, 1, 1);
        current_line = 1;
        mvwprintw(data_win, current_line, 1, "L %d dh %d sh %d eh %d\n", 
                  LINES, data_height, stat_height, err_height);
        current_line++;
        mvwprintw(data_win, current_line, 1, 
                  "Started listening in port %d...\n", port);
        current_line++;
        mvwprintw(data_win, current_line, 1, "Expecting %d nodes...\n", nodes);

        rwrefresh(data_win, "Data");
        
        while (1 > 0) {
            if (recvfrom(s, 
                         buf,
                         BUFLEN, 
                         0, 
                         (struct sockaddr *)&si_other, 
                         &slen)==-1) {
                perror("recvfrom()");
            }
            time(&timer); 
            tm_info = localtime(&timer);
            strftime(time_buffer, 25, "%H:%M:%S", tm_info);
            /* scroll if needed */
            current_line++;
            if (current_line > data_height - BOTTOM) {
                current_line = data_height - BOTTOM;
                scroll(data_win);
            } 
            mvwprintw(data_win, current_line, 1, 
                      "%s From %s:%d Data: %s", time_buffer, 
                      inet_ntoa(si_other.sin_addr),  
                      ntohs(si_other.sin_port), buf);
            
            found = 0;
            for (i = 0; i < nodes; i++) {
                if (mep_stats[i].mepid == atoi(&buf[8])) {
                    mep_stats[i].nmsg++;
                    found = 1;
                }
            }
            if (found == 0) {
                mep_stats[seen_nodes].mepid = atoi(&buf[8]);
                mep_stats[seen_nodes].nmsg++;
                seen_nodes++;
            }
            
            memset(min_buffer, '0', sizeof(min_buffer));
            memset(mean_buffer, '0', sizeof(mean_buffer));             
            memset(max_buffer, '0', sizeof(max_buffer));
            
            if (strstr(buf, "STAT") != 0) {
                where = strstr(buf, "::");
                where = where + 1;
                where2 = strstr(buf, ":::");
                start = (where - buf) + 1;
                howmany= where2 - where - 1;
                snprintf(min_buffer, howmany, "%s\n", &buf[start]);  
                where = strstr(buf, ":::");
                where = where + 1;
                where2 = strstr(buf, "::::");
                start = (where - buf) + 2;
                howmany= where2 - where - 1;
                snprintf(mean_buffer, howmany, "%s\n", &buf[start]);  
                where = strstr(buf, "::::");
                where = where + 1;
                where2 = strstr(buf, ":::::");
                start = (where - buf) + 3;
                howmany= where2 - where - 1;
                snprintf(max_buffer, howmany, "%s\n", &buf[start]);  
                for (i = 0; i < nodes; i++) {
                    if (mep_stats[i].mepid == atoi(&buf[8])) {
                        mep_stats[i].min = atof(min_buffer);
                        mep_stats[i].mean = atof(mean_buffer);
                        mep_stats[i].max = atof(max_buffer);
                    }
                }
            } else if (strstr(buf, "CCM ALARM for") != 0) {
                if (has_colors()) wbkgd(err_win, COLOR_PAIR(ERROR));
                for (i = 0; i < nodes; i++) {
                    if (mep_stats[i].mepid == atoi(&buf[8])) {
                        mep_stats[i].rdi++;
                    }
                }
                for (i = 0; i < nodes; i++) {
                    if (mep_stats[i].mepid == atoi(&buf[32])) {
                        mep_stats[i].alarm++;
                    }
                }
            } else if (strstr(buf, "lowered") != 0) {
                if (has_colors()) wbkgd(err_win, COLOR_PAIR(PLAIN));
                for (i = 0; i < nodes; i++) {
                    if (mep_stats[i].mepid == atoi(&buf[8])) {
                        mep_stats[i].rdi--;
                    }
                }
                for (i = 0; i < nodes; i++) {
                    if (mep_stats[i].mepid == atoi(&buf[50])) {
                        mep_stats[i].alarm--;
                    }
                }
            } else if (strstr(buf, "warning limit") != 0) {
                for (i = 0; i < nodes; i++) {
                    if (mep_stats[i].mepid == atoi(&buf[17])) {
                        mep_stats[i].warnings++;
                    }
                }
            } else if (strstr(buf, "error limit") != 0) {
                for (i = 0; i < nodes; i++) {
                    if (mep_stats[i].mepid == atoi(&buf[17])) {
                        mep_stats[i].errors++;
                    }
                }
            }
            
            for (i = 0; i < nodes; i++) {
                if (mep_stats[i].mepid == 0) continue;
                mvwprintw(stat_win, (i + 1), 1, 
                          "ID %d, #msgs %d, min %f, mean %f, max %f", 
                          mep_stats[i].mepid, 
                          mep_stats[i].nmsg,
                          mep_stats[i].min,
                          mep_stats[i].mean,
                          mep_stats[i].max);
            }
            
            for (i = 0; i < nodes; i++) {
                if (mep_stats[i].mepid == 0) continue;
                mvwprintw(err_win, (i + 1), 1, 
                          "ID %d, #warnings %d, #errors %d, alarm %d rdi %d\n",
                          mep_stats[i].mepid,
                          mep_stats[i].warnings, 
                          mep_stats[i].errors,
                          mep_stats[i].alarm,
                          mep_stats[i].rdi);
            }
            refresh_all_windows();
      
            if (gnuplot == 1 && nodes >= 2) {
                /* open file for appending if the file 
                   does not exist create it */
                f = fopen("stats.dat", "a");
                /* print line */
                time(&timer); 
                //tm_info = localtime(&timer);
                //strftime(time_buffer, 25, "%H%M%S", tm_info);
                fprintf(f, "%d \t %f \t %f\n", 
                        (int)timer, mep_stats[0].mean, mep_stats[1].mean);
                /* flush */
                fflush(f);
                /* close */
                fclose(f);
            }
        }
        delwin(err_win);
        delwin(data_win);
        endwin();
        refresh();
    } else {
        /* NO CURSES MODE */
        printf("Started listening in port %d...\n", port);
        printf("Expecting %d nodes...\n", nodes);
        while (1 > 0) {
            if (recvfrom(s, 
                         buf,
                         BUFLEN, 
                         0, 
                         (struct sockaddr *)&si_other, 
                         &slen)==-1) {
                perror("recvfrom()");
            }
            time(&timer);
            tm_info = localtime(&timer);
            strftime(time_buffer, 25, "%H:%M:%S", tm_info);
            printf("%s From %s:%d Data: %s", time_buffer, 
                   inet_ntoa(si_other.sin_addr),  
                   ntohs(si_other.sin_port), buf);
        }
    }
    close(s);
    return 0;
}
