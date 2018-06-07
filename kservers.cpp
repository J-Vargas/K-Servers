//============================================================================
// Name        : kserver.cpp
// Author      : Yamile Vargas 
// Copyright   : 
// Description : Executes an online and offline algorithm for 3 servers and n points.
//============================================================================

#include <algorithm>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <stack>
#include <queue>
#include <stdlib.h>
using namespace std;

#define INFINITE 9999999
#define YELLOW 0
#define RED 1
#define BLUE 2

/*The requests*/
typedef struct st_request {
	double x;
	double y;
	} request;

typedef struct st_configuration {
	int time;
	int  yellow;
	int  red;
	int  blue;
	int server_moved;
	} configuration;

int  number_requests = 0;
double ****cost;
configuration ****paths;
request requests[1000] = {};
request configuration0[3] = {};
stack <configuration> cheapest_cost_path;
double distance_traveled_online = 0;
double distance_traveled_offline;
int count_requests = 1; //at request 0 all the servers are in its original position.

/*WINDOW DISPLAY: starts*/
Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;
Window win;
int border_width, win_x, win_y;
unsigned int win_width, win_height;
XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "K_SERVERS - Hw02 - YPV";
char *icon_name_string = "Icon for Example Window";

XEvent report;

GC gc, gc_yellow, gc_red, gc_blue, gc_gray, gc_tred, gc_tyellow, gc_tblue;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_blue_values, gc_gray_values;
XGCValues gc_tyellow_values, gc_tred_values, gc_tblue_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

void setColors(Display* display_ptr, Window	win){
	gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
	XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
	XSetLineAttributes( display_ptr, gc, 4, LineSolid, CapRound, JoinRound);

	/* and three other graphics contexts, to draw in yellow and red and grey white=WhitePixel(dis, screen);*/
	gc_yellow = XCreateGC( display_ptr, win, valuemask, &gc_yellow_values);
	XSetLineAttributes(display_ptr, gc_yellow, 1, LineSolid,CapRound, JoinRound);
	gc_tyellow = XCreateGC( display_ptr, win, valuemask, &gc_tyellow_values);
	XSetLineAttributes(display_ptr, gc_tyellow, 2, LineSolid,CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "yellow", &tmp_color1, &tmp_color2 ) == 0 )
	{printf("failed to get color yellow\n"); exit(-1);}
	else{
		XSetForeground( display_ptr, gc_yellow, tmp_color1.pixel );
		XSetForeground( display_ptr, gc_tyellow, tmp_color1.pixel );
	}

	/* other graphics contexts red*/
	gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
	XSetLineAttributes( display_ptr, gc_red, 1, LineSolid, CapRound, JoinRound);
	gc_tred = XCreateGC( display_ptr, win, valuemask, &gc_tred_values);
	XSetLineAttributes( display_ptr, gc_tred, 2, LineSolid, CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "red", &tmp_color1, &tmp_color2 ) == 0 )
	{printf("failed to get color red\n"); exit(-1);}
	else
	{XSetForeground( display_ptr, gc_tred, tmp_color1.pixel );
	XSetForeground( display_ptr, gc_red, tmp_color1.pixel );
	}
	/* other graphics contexts red*/

	gc_gray = XCreateGC( display_ptr, win, valuemask, &gc_gray_values);
	XSetLineAttributes( display_ptr, gc_gray, 2, LineSolid, CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "light grey", &tmp_color1, &tmp_color2 ) == 0 )
	{printf("failed to get color gray\n"); exit(-1);}
	else
		XSetForeground( display_ptr, gc_gray, tmp_color1.pixel );

	/*other graphics contexts grey*/
	gc_blue = XCreateGC( display_ptr, win, valuemask, &gc_blue_values);
	XSetLineAttributes( display_ptr, gc_blue, 1, LineSolid, CapRound, JoinRound);
	gc_tblue = XCreateGC( display_ptr, win, valuemask, &gc_tblue_values);
	XSetLineAttributes( display_ptr, gc_tblue, 2, LineSolid, CapRound, JoinRound);
	if( XAllocNamedColor( display_ptr, color_map, "blue", &tmp_color1, &tmp_color2 ) == 0 )
	{printf("failed to get color blue\n"); exit(-1);}
	else{
		XSetForeground( display_ptr, gc_blue, tmp_color1.pixel );
		XSetForeground( display_ptr, gc_tblue, tmp_color1.pixel );
	}
}

void allocateSpaceForMatrices(int count_requests){
	int i, j, k; i = j = k =0;
	 cost = new double***[count_requests];
	 paths = new configuration***[count_requests];
	 for(i = 0; i < count_requests; i++){
		 cost[i] = new double**[count_requests];
		 paths[i] = new configuration**[count_requests];
		 for(j = 0; j < count_requests; j++){
			 cost[i][j] = new double*[count_requests];
			 paths[i][j] = new configuration*[count_requests];
			 for(k = 0; k < count_requests; k++){
				 cost[i][j][k] = new double[count_requests];
				 paths[i][j][k] = new configuration[count_requests];
			 }
		 }
	 }
}

void deallocateSpaceForMatrices(int count_requests){
	int i, j, k; i = j = k =0;
	 for(i = 0; i < count_requests; i++){
		 for(j = 0; j < count_requests; j++){
			 for(k = 0; k < count_requests; k++){
				 cost[i][j][k] = 0;
				 paths[i][j][k] = 0;
			 }
			 cost[i][j] = 0;
			 paths[i][j] = 0;
		 }
		 cost[i] = 0;
		 paths[i] = 0;
	 }
	 cost = 0;
	 paths = 0;
}

void set_initial_pos_servers(unsigned int  win_width, unsigned int win_height){
	configuration0[YELLOW].x = 1;
	configuration0[YELLOW].y = 1;
	configuration0[RED].x = win_width/1.005;
	configuration0[RED].y = 1;
	configuration0[BLUE].x = (win_width -win_x)/2;
	configuration0[BLUE].y = win_height/1.005;
}
/*Distance between two points*/
double distance_p(request p1, request p2){
	double l = (sqrt( pow( p2.x - p1.x, 2 ) + pow(p2.y - p1.y, 2 )));
	return l;
}

void print_path(stack <configuration> a_path){
	cout << "\n"; cout << " Cheapest Cost Path :" << "\n";
	string  serv[1] = {};
	while(!a_path.empty()){
		if (a_path.top().time ==0)
			cout <<"Servers configuration at request "<<a_path.top().time << ":  [" << a_path.top().yellow<<"]["<<a_path.top().red<<"]["<<a_path.top().blue<<"] ->Initial Configuration." << endl ;
		else
			cout << "Servers configuration at request "<<a_path.top().time <<  ":  [" << a_path.top().yellow<<"]["<<a_path.top().red<<"]["<<a_path.top().blue<<"] -> server moved: "<< serv[0] << endl ;

		serv[0]= a_path.top().server_moved == BLUE? "Blue": a_path.top().server_moved == RED? "Red": "Yellow";
		a_path.pop();
	}
	cout << "\n";
}

void draw_path( stack <configuration> path){
	configuration temp;
	if(!path.empty()){
		temp = path.top();
		path.pop();
		while(!path.empty()){
			if(temp.server_moved == YELLOW){
				if(temp.yellow == 0)
					XDrawLine(display_ptr, win, gc_tyellow, configuration0[YELLOW].x, configuration0[YELLOW].y, requests[path.top().yellow].x,  requests[path.top().yellow].y);
				else
					XDrawLine(display_ptr, win, gc_tyellow, requests[temp.yellow].x, requests[temp.yellow].y, requests[path.top().yellow].x,  requests[path.top().yellow].y);
			}
			else if(temp.server_moved == RED){
				if(temp.red == 0)
					XDrawLine(display_ptr, win, gc_tred,configuration0[RED].x, configuration0[RED].y,requests[path.top().red].x,  requests[path.top().red].y);
				else
					XDrawLine(display_ptr, win, gc_tred, requests[temp.red].x, requests[temp.red].y, requests[path.top().red].x,  requests[path.top().red].y);
			}else{ // if(temp.server_moved == BLUE)
				if(temp.blue == 0)
					XDrawLine(display_ptr, win, gc_tblue, configuration0[BLUE].x, configuration0[BLUE].y, requests[path.top().blue].x,  requests[path.top().blue].y);
				else
					XDrawLine(display_ptr, win, gc_tblue, requests[temp.blue].x, requests[temp.blue].y, requests[path.top().blue].x,  requests[path.top().blue].y);
			}
			temp = path.top();
			path.pop();
		}
	}
}

void DisplayConfiguration(Display* display_ptr, Drawable win, GC gc_r){
	XDrawRectangle(display_ptr, win, gc_tyellow, configuration0[YELLOW].x, configuration0[YELLOW].y,3,3);
	XFillRectangle(display_ptr, win, gc_tyellow, configuration0[YELLOW].x, configuration0[YELLOW].y,3,3);
	XDrawRectangle(display_ptr, win, gc_tred, configuration0[RED].x, configuration0[RED].y,3,3);
	XFillRectangle(display_ptr, win, gc_tred, configuration0[RED].x, configuration0[RED].y,3,3);
	XDrawRectangle(display_ptr, win, gc_tblue, configuration0[BLUE].x, configuration0[BLUE].y,3,3);
	XFillRectangle(display_ptr, win, gc_tblue, configuration0[BLUE].x, configuration0[BLUE].y,3,3);
}

/*Sends the closest server to the requested position*/
double sendserver(request r){
	int min; double temp_distance = INFINITE;
	GC temp_gc;
	if(distance_p(configuration0[YELLOW],r) <= distance_p(configuration0[RED],r)){
		min = YELLOW;
		temp_gc = gc_yellow;
		temp_distance = distance_p(configuration0[YELLOW],r) ;
	}else{
		min = RED;
		temp_gc = gc_red;
		temp_distance = distance_p(configuration0[RED],r) ;
	}
	if(distance_p(configuration0[BLUE],r) <= distance_p(configuration0[min],r)){
		min = BLUE;
		temp_gc = gc_blue;
		temp_distance = distance_p(configuration0[BLUE],r) ;
	}
	XDrawLine(display_ptr, win, temp_gc, configuration0[min].x, configuration0[min].y, r.x, r.y);
	configuration0[min] = r;
	XDrawRectangle(display_ptr, win, temp_gc, r.x, r.y,3,3);  XFillRectangle(display_ptr, win, temp_gc, r.x, r.y,3,3);
	return temp_distance;
}

/*
 * finds the optimal strategy by a dynamic-programming
 * */
void compute_optimal_stratey(){
	int  t, yellow_server, red_sever, blue_server,  request_i;
	int temp_t, temp_yellow, temp_red, temp_blue, temp_server;
	int t_min_cost , conf_yellow_min_cost, conf_red_min_cost, conf_blue_min_cost;
	double path_with_min_cost = INFINITE;
	double temp_min_cost = INFINITE;
	t = yellow_server = red_sever = blue_server = request_i = 0;
	temp_t = temp_yellow = temp_red = temp_blue = temp_server = INFINITE;
	t_min_cost = conf_yellow_min_cost = conf_red_min_cost  = conf_blue_min_cost = INFINITE;
	//setting all offline_paths to INIFINITE
	for( t=0; t<=number_requests; t++){
		for( yellow_server=0; yellow_server<=number_requests; yellow_server++){
			for( red_sever=0; red_sever<=number_requests; red_sever++){
				for( blue_server=0; blue_server<=number_requests; blue_server++){
					cost[t][yellow_server][red_sever][blue_server]= INFINITE;
	}}}}

	/*Calculate all possible paths*/
	cost[0][0][0][0] = 0;

	for( t=1; t<=number_requests; t++){
		path_with_min_cost = INFINITE;
		for( yellow_server=0; yellow_server<=t; yellow_server++){
			for( red_sever=0; red_sever<=t; red_sever++){
				if(!(yellow_server != 0 and yellow_server == red_sever) ){
				for( blue_server=0; blue_server<=t; blue_server++){
					if(!(blue_server != 0 and (yellow_server == blue_server or red_sever == blue_server) )){
					if(yellow_server == t or red_sever == t or blue_server == t){
						for(request_i = 0; request_i < t; request_i++){
							if(blue_server == t and cost[t-1][yellow_server][red_sever][request_i] != INFINITE){
								if(request_i == 0)
									temp_min_cost =  distance_p(configuration0[BLUE],requests[t]) + cost[t-1][yellow_server][red_sever][request_i];
								else
									temp_min_cost =  distance_p(requests[request_i],requests[t]) + cost[t-1][yellow_server][red_sever][request_i];
								temp_t = t-1; temp_yellow = yellow_server; temp_red = red_sever;temp_blue = request_i; temp_server = BLUE;
							}
							if(red_sever == t and cost[t-1][yellow_server][request_i][blue_server] != INFINITE){
								if(request_i == 0)
									temp_min_cost =  distance_p(configuration0[RED],requests[t]) + cost[t-1][yellow_server][request_i][blue_server];
								else
									temp_min_cost = distance_p(requests[request_i],requests[t]) + cost[t-1][yellow_server][request_i][blue_server];
								temp_t = t-1; temp_yellow = yellow_server; temp_red = request_i; temp_blue = blue_server; temp_server = RED;
							}
							if(yellow_server==t and cost[t-1][request_i][red_sever][blue_server] != INFINITE){
								if(request_i == 0)
									temp_min_cost =  distance_p(configuration0[YELLOW],requests[t]) + cost[t-1][request_i][red_sever][blue_server];
								else
									temp_min_cost =  distance_p(requests[request_i],requests[t]) + cost[t-1][request_i][red_sever][blue_server];
								temp_t = t-1; temp_yellow = request_i; temp_red = red_sever; temp_blue = blue_server;  temp_server = YELLOW;
							}
							if(temp_min_cost < cost[t][yellow_server][red_sever][blue_server] ){
								cost[t][yellow_server][red_sever][blue_server]  = temp_min_cost;
								paths[t][yellow_server][red_sever][blue_server].time = temp_t;
								paths[t][yellow_server][red_sever][blue_server].yellow = temp_yellow;
								paths[t][yellow_server][red_sever][blue_server].red = temp_red;
								paths[t][yellow_server][red_sever][blue_server].blue = temp_blue;
								paths[t][yellow_server][red_sever][blue_server].server_moved = temp_server;
								 temp_min_cost =temp_t = temp_yellow = temp_red = temp_blue = temp_server = INFINITE;
							}
						}
						temp_min_cost = INFINITE;
						if(cost[t][yellow_server][red_sever][blue_server]  != INFINITE and cost[t][yellow_server][red_sever][blue_server]  < path_with_min_cost){
							path_with_min_cost= cost[t][yellow_server][red_sever][blue_server] ;
							t_min_cost = t;
							conf_yellow_min_cost=yellow_server;
							conf_red_min_cost = red_sever;
							conf_blue_min_cost = blue_server;
						}
					}
				}}}}}
	}
	/*---- Build a stack with the configurations of the servers for the cheapest path*/

	/* distance_traveled_offline is the  value of the cheapest path to get to the last request.*/
	distance_traveled_offline = path_with_min_cost;
	/*temp_conf initially stores the last configuration of the servers for the cheapest path*/
	configuration temp_conf ;
	temp_conf.time = t_min_cost; temp_conf.yellow = conf_yellow_min_cost; temp_conf.red = conf_red_min_cost; temp_conf.blue = conf_blue_min_cost; temp_conf.server_moved = INFINITE;
	cheapest_cost_path.push(temp_conf);
	while(temp_conf.time >  0){
		temp_conf = paths[cheapest_cost_path.top().time][cheapest_cost_path.top().yellow][cheapest_cost_path.top().red][cheapest_cost_path.top().blue];
		cheapest_cost_path.push(temp_conf);
	}
//	print_path(cheapest_cost_path);
	draw_path(cheapest_cost_path);
}

int main(int argc, char *argv[])
{
	int right_click = 0;
	/* opening display: basic connection to X Server */
	if( (display_ptr = XOpenDisplay(display_name)) == NULL ){printf("Could not open display. \n"); exit(-1);}
	screen_num = DefaultScreen( display_ptr );
	screen_ptr = DefaultScreenOfDisplay( display_ptr );
	color_map  = XDefaultColormap( display_ptr, screen_num );
	display_width  = DisplayWidth( display_ptr, screen_num );
	display_height = DisplayHeight( display_ptr, screen_num );
	/* creating the window */
	border_width = 10;
	win_x = 25; win_y = 25; win_width = display_width/1.2; win_height = (int) (win_width /1.7); /*rectangular window*/

	win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
			win_x, win_y, win_width, win_height, border_width,
			BlackPixel(display_ptr, screen_num),
			WhitePixel(display_ptr, screen_num) );

	/* now try to put it on screen, this needs cooperation of window manager */
	size_hints = XAllocSizeHints();
	wm_hints = XAllocWMHints();
	class_hints = XAllocClassHint();
	if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
	{ printf("Error allocating memory for hints. \n"); exit(-1);}

	size_hints -> flags = PPosition | PSize | PMinSize  ;
	size_hints -> min_width = 60;
	size_hints -> min_height = 60;

	XStringListToTextProperty( &win_name_string,1,&win_name);
	XStringListToTextProperty( &icon_name_string,1,&icon_name);

	wm_hints -> flags = StateHint | InputHint ;
	wm_hints -> initial_state = NormalState;
	wm_hints -> input = False;

	class_hints -> res_name = "x_use_example";
	class_hints -> res_class = "examples";

	XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc, size_hints, wm_hints, class_hints );

	/* what events do we want to receive */
	XSelectInput( display_ptr, win, ExposureMask | StructureNotifyMask | ButtonPressMask );

	/* finally: put window on screen */
	XMapWindow( display_ptr, win );
	XFlush(display_ptr);

	/* create graphics context, so that we may draw in this window */
	setColors(display_ptr,win);
	/*Define Initial position of servers*/
	set_initial_pos_servers(win_width,win_height);

	while(1)
	{
		XNextEvent( display_ptr, &report );
		switch( report.type )
		{
		case Expose:
			DisplayConfiguration(display_ptr, win, gc_gray);
			break;
		case ConfigureNotify:
			/* This event happens when the user changes the size of the window*/
			win_width = report.xconfigure.width;
			win_height = report.xconfigure.height;
			break;
		case ButtonPress:
		{
			/*
			 * implement strategy to decide what server to send to this point(x, y)
			 * display the "path" from the position of the server to the request
			 * update the position of the server to the new position (x, y)
			 * */
			int x, y;
			x = report.xbutton.x;
			y = report.xbutton.y;
			if (report.xbutton.button == Button1){
				if(right_click ==0 ){
					requests[count_requests].x = x;
					requests[count_requests].y = y;
					distance_traveled_online = distance_traveled_online + sendserver(requests[count_requests]);
					count_requests++;
				}
			}else{ //right click
				/*
				 * - It stops the requests to the servers
				 * - Computes and display the optimal strategy
				 * - Prints (to stdout) the total length of both strategies,
				 * - Prints the competitiveness ratio that was achieved.
				 */
				right_click++;
				if (right_click == 2){
					if( count_requests > 0){
						number_requests = count_requests -1;
						allocateSpaceForMatrices(count_requests);
						set_initial_pos_servers(win_width,win_height);
						compute_optimal_stratey();
						printf("Total distance with Online algorithm: %f \n", distance_traveled_online);
						printf("Total distance with Offline algorithm: %f \n", distance_traveled_offline);
						printf("Competitiveness Ratio: %f %\n", (distance_traveled_offline/distance_traveled_online)*100);
						deallocateSpaceForMatrices(count_requests);
					}else
						printf("There were zero requests\n");
				}
				if(right_click > 2){
					XFreeGC(display_ptr, gc);
					XFreeGC(display_ptr, gc_gray);
					XFreeGC(display_ptr, gc_yellow);
					XFreeGC(display_ptr, gc_red);
					XDestroyWindow(display_ptr,win);
					XCloseDisplay(display_ptr);
				}
			}
		}
		break;
		default:
			/* this is a catch-all for other events; it does not do anything.
              One could look at the report type to see what the event was */
			break;
		}

	}

	exit(0);
}
