#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>


struct Cell {
    bool alive;
    int neighbours;
};

// util to give titel to window
void print_title_to_center(WINDOW *win, char *title){
    int center_col = getmaxx(win) / 2;
    int half_length = strlen(title) / 2;
    int adjusted_col = center_col - half_length;
    mvwprintw(win, 0, adjusted_col, title);
}


int main(int argc, char** argv){
    setlocale(LC_ALL,"");
    initscr();
    cbreak();
    nodelay(stdscr, true);
    noecho();
    keypad(stdscr, true);
    curs_set(false);

    // setup colors for red and green
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    
    // get terminal dimenions
    int TERM_HEIGHT, TERM_WIDTH;
    getmaxyx(stdscr, TERM_HEIGHT, TERM_WIDTH);

    WINDOW *SIM = newwin(TERM_HEIGHT, TERM_WIDTH - 30, 0, 0);
    refresh();
    box(SIM, 0, 0);
    print_title_to_center(SIM, "Simulation");
    wrefresh(SIM);

    WINDOW *STATS = newwin(10, 30, 0, TERM_WIDTH - 30);
    refresh();
    box(STATS, 0, 0);
    print_title_to_center(STATS, "Stats");
    wrefresh(STATS);

    // prepare for mouse input
    mousemask(ALL_MOUSE_EVENTS ,NULL);
    bool mouse_click = false;
    MEVENT event;
    
    //---Stats---

    int alive = 0;      // number of alive cells
    int mouse_x = -1;
    int mouse_y = -1;
    bool run = false;
    
    //---Simulation---
    
    const int SIM_WIDTH = getmaxx(SIM)-2;
    const int SIM_HEIGHT = getmaxy(SIM)-2;
    struct Cell sim_arr [SIM_HEIGHT][SIM_WIDTH];
    
    for(size_t i = 0; i < SIM_HEIGHT; i++){
        for(size_t j = 0; j < SIM_WIDTH; j++){
            sim_arr[i][j].alive = false;
            sim_arr[i][j].neighbours = 0;
        }
    }

    // debugging glider
    sim_arr[0][0].alive = true;
    sim_arr[1][1].alive = true;
    sim_arr[1][2].alive = true;
    sim_arr[2][0].alive = true;
    sim_arr[2][1].alive = true;
    

    // main loop
    while(true){
        int ch = getch();
        switch (ch) {
            case 'q':
                goto end;

            case 'r':
                run = !run;
                break;
            
            case KEY_MOUSE:
                if (getmouse(&event) == OK){
                    if(event.bstate & BUTTON1_CLICKED){
                        mouse_x = event.x;
                        mouse_y = event.y;
                        mouse_click = true;
                    }
                }
                break;
            default:
                break;
        }
        
        /////////////////////
        //---Simulation---///
        /////////////////////

        // check if cell clicked
        if(mouse_click){
            if(mouse_x <= SIM_WIDTH && mouse_y <= SIM_HEIGHT){
                sim_arr[mouse_y-1][mouse_x-1].alive = !sim_arr[mouse_y-1][mouse_x-1].alive;
                mouse_click = false;
            }
        }
        // calculate neighbours
        alive = 0;
        for(int i=0; i < SIM_HEIGHT; i++){
            for(int j=0; j < SIM_WIDTH; j++){
                alive += sim_arr[i][j].alive;
                sim_arr[i][j].neighbours = sim_arr[(((i) % SIM_HEIGHT) + SIM_HEIGHT) % SIM_HEIGHT][(((j+1) % SIM_WIDTH) + SIM_WIDTH) % SIM_WIDTH].alive
                                         + sim_arr[(((i+1) % SIM_HEIGHT) + SIM_HEIGHT) % SIM_HEIGHT][(((j+1) % SIM_WIDTH) + SIM_WIDTH) % SIM_WIDTH].alive
                                         + sim_arr[(((i+1) % SIM_HEIGHT) + SIM_HEIGHT) % SIM_HEIGHT][(((j) % SIM_WIDTH) + SIM_WIDTH) % SIM_WIDTH].alive
                                         + sim_arr[(((i+1) % SIM_HEIGHT) + SIM_HEIGHT) % SIM_HEIGHT][(((j-1) % SIM_WIDTH) + SIM_WIDTH) % SIM_WIDTH].alive
                                         + sim_arr[(((i) % SIM_HEIGHT) + SIM_HEIGHT) % SIM_HEIGHT][(((j-1) % SIM_WIDTH) + SIM_WIDTH) % SIM_WIDTH].alive
                                         + sim_arr[(((i-1) % SIM_HEIGHT) + SIM_HEIGHT) % SIM_HEIGHT][(((j-1) % SIM_WIDTH) + SIM_WIDTH) % SIM_WIDTH].alive
                                         + sim_arr[(((i-1) % SIM_HEIGHT) + SIM_HEIGHT) % SIM_HEIGHT][(((j) % SIM_WIDTH) + SIM_WIDTH) % SIM_WIDTH].alive
                                         + sim_arr[(((i-1) % SIM_HEIGHT) + SIM_HEIGHT) % SIM_HEIGHT][(((j+1) % SIM_WIDTH) + SIM_WIDTH) % SIM_WIDTH].alive;

                                        
            }
        }

        // do calculation
        if(run){
            for(size_t i=0; i < SIM_HEIGHT; i++){
                for(size_t j=0; j < SIM_WIDTH; j++){
                    if(sim_arr[i][j].alive && sim_arr[i][j].neighbours < 2){
                        sim_arr[i][j].alive = false;
                    }
                    else if(sim_arr[i][j].alive && (sim_arr[i][j].neighbours == 2 || sim_arr[i][j].neighbours == 3)){
                        sim_arr[i][j].alive = true;
                    }
                    else if(sim_arr[i][j].alive && sim_arr[i][j].neighbours > 3){
                        sim_arr[i][j].alive = false;
                    }
                    else if((!sim_arr[i][j].alive) && sim_arr[i][j].neighbours == 3){
                        sim_arr[i][j].alive = true;
                    }
                }
            }
        }
        


        // render each cell
        for(size_t i = 0; i < SIM_HEIGHT; i++){
            for(size_t j = 0; j<SIM_WIDTH; j++){
                if(sim_arr[i][j].alive){
                    mvwprintw(SIM, i+1, j+1, "#");
                }
                else{
                    mvwprintw(SIM, i+1, j+1, " ");
                }
                wrefresh(SIM);
            }
        }

        /////////////////////
        //---Statistics---///
        /////////////////////

        // if simulation running
        mvwprintw(STATS, 1, 1, "Simulation: ");
        if(run){
            wattron( STATS, COLOR_PAIR(2));
            mvwprintw(STATS, 1, strlen("Simulation: ")+1, "Running");
            wattroff(STATS, COLOR_PAIR(2));
        }
        else{
            wattron( STATS, COLOR_PAIR(1));
            mvwprintw(STATS, 1, strlen("Simulation: ")+1, "Stopped");
            wattroff(STATS, COLOR_PAIR(1));
        }
        // mouse location
        wmove(STATS, 2, 1);
        wclrtoeol(STATS);
        mvwprintw(STATS, 2, 1, "Mouse loc: ( %d, %d)", mouse_x, mouse_y);
        
        // no. of alive cells
        wmove(STATS, 3, 1);
        wclrtoeol(STATS);
        mvwprintw(STATS, 3, 1, "No. of alive cells: %d", alive);
        box(STATS,0,0);
        print_title_to_center(STATS, "Stats");
        wrefresh(STATS);
    }
    end: 
    endwin();
    return EXIT_SUCCESS;
}
