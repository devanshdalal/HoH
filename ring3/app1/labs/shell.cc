#include "labs/shell.h"
#include "util/debug.h"
#include "labs/vgatext.h"

#include "string.h"
#define BI 9
//
// initialize shellstate
//
void shell_init(shellstate_t& state){
    state.kcount = state.i1  = state.cstate = 0;
    state.option=1;
    state.inp[0]=state.out[0]='\0';
    state.changed =1 ;
    // indexes       {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13,  14,  15,   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,   48, 49, 50, 51, 52, 53, 54};
    char mymap[]=    {'?','~','1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',  'q','w','e','r','t','y','u','i','o','p','[',']','?','?','a','s',  'd','f','g','h','j','k','l',';','?','?','?','?','z','x','c','v',  'b','n','m',',','.','/','?'};
    state.SIZ=sizeof(mymap)/sizeof(char);
    for (int i = 0; i < state.SIZ; ++i)
        state.TAB[i]=mymap[i];
    state.f_done=1;
    state.f_call=0;
    state.fibr_done=1;
    state.fibr_call=0;
    state.lmt=10;
    state.fsc_c=0;
    state.turn=0;
    for (int i = 1; i < 10; ++i)
    {
        state.fs_done[i]=1;
        state.fs_call[i]=0;
    }
    for (int i = 1; i <=5; ++i)
    {
        state.fs_charo[i][0]='\0';
        state.fs_chari[i][0]='\0';
        state.fs_len[i]=0;
        state.slotfr[i]=1;
    }
    state.pcount=0;
    hoh_debug("DIRECTIONS:\n1.Use up or down to navigate.\n2.Right arrow to select highlighted command.\n3.Type input & hit Enter.\n4.Out will be displayed.\n4.Goto step 1.\n" );
}

//
// handle keyboard event.
// key is in scancode format.
// For ex:
// scancode for following keys are:
//
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | keys     | esc |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 |  0 |  - |  = |back|
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | scancode | 01  | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0a | 0b | 0c | 0d | 0e |
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | keys     | tab |  q |  w |  e |  r |  t |  y |  u |  i |  o |  p |  [ |  ] |entr|
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | scancode | 0f  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1a | 1b | 1c |
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | keys     |ctrl |  a |  s |  d |  f |  g |  h |  j |  k |  l |  ; |  ' |    |shft|
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | scancode | 1d  | 1e | 1f | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 2a |
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//
// so and so..
//
// - restrict yourself to: 0-9, a-z, esc, enter, arrows
// - ignore other keys like shift, control keys
// - only handle the keys which you're interested in
// - for example, you may want to handle up(0x48),down(0x50) arrow keys for menu.
//

int getslot(shellstate_t & state){
	for (int i = 1; i <=5; ++i){
		if (state.slotfr[i])return i;
	}
	return -1;
}

void shell_update(uint8_t scankey, shellstate_t& state){
    state.kcount++;
    if (scankey==1){
        int keypressed=state.kcount;
        shell_init(state);
        state.kcount=keypressed;
    }
    switch(state.cstate){
        case 0:
            if(scankey==0x48){
                ///  up arrow
                if((--state.option)==0)state.option=BI-1;
            }else if(scankey==0x50){
                ///  down arrow
                state.option=(state.option)%(BI-1)+1;
            }else if (state.option<=6 and (state.f_call!=2 or state.option!=5) and (state.fibr_call!=2 or state.option!=6) and scankey==0x1c)
            {
                /* enter  */
                state.cstate++;
                state.currfs=0;
            }else if (state.option>6 and scankey==0x1c)
            {
            	/* enter long task */
            	state.currfs=getslot(state);
            	if (state.currfs==-1)
            	{
            		strcpy(state.out,"No slot available");
            	}else{
            		state.slotfr[state.currfs]=0;
            		state.fs_chari[state.currfs][state.fs_len[state.currfs]=0]='\0';
            		state.cstate++;
            	}
            }
            state.inp[state.i1=0]='\0';
            break;
        case 1:            
            if(state.option<=6 and scankey!=0x1c){
                ///  not enter
                if (scankey==0xe){
	                // backspace
	                if(state.i1>0)state.inp[--state.i1]='\0';
	            }else{
	                char ch = (0<=scankey and scankey<state.SIZ)?state.TAB[scankey]: (scankey==0x39 ? ' ':char(238));
	                if (ch!=char(238) and state.i1<=55){
	                    state.inp[state.i1++]=ch;  /// assuming a-z, ' ' or 0-9 are pressed.
	                    state.inp[state.i1]='\0'; 
	                }
	            }
            }
            else if (state.option>6 and scankey!=0x1c)
            {
            	///  not enter
            	if (scankey==0xe){
	                // backspace
                    if(state.fs_len[state.currfs]>0)state.fs_chari[state.currfs][--state.fs_len[state.currfs]]='\0';
                }else{
                    char ch = (0<=scankey and scankey<state.SIZ)?state.TAB[scankey]: (scankey==0x39 ? ' ':char(238));
                    if (ch!=char(238) and state.fs_len[state.currfs]<10){
                        state.fs_chari[state.currfs][state.fs_len[state.currfs]++]=ch;  /// assuming a-z, ' ' or 0-9 are pressed.
                        state.fs_chari[state.currfs][state.fs_len[state.currfs]  ]='\0'; 
	                }	
	            }
            }else{
                /* enter  */
                state.cstate++;
                if(state.option>6)state.pcount++;
            }
            break;
        case 2:
            state.cstate=0;
        default:;
    }
    hoh_debug("Got: "<<unsigned(scankey));
    // increment the
}

//
// do computation
//

int fib(int n){													/// FIBONACCI 
    if (n<=2)return (n>0);
    return fib(n-1)+fib(n-2);
}

int fact(int n){												/// FACTORIAL
    if (n<=1)return 1;
    int ret=n*fact(n-1);
    return ret>0?ret:-ret;
}

int kthprime(int k){											/// KTH PRIME
    for (int n=2; k>0 ;n++){
        bool prime=1;
        for (int i = 2; i*i<=n ; ++i)
            if (n%i==0){
                prime=0;
                break;
            }
        if (prime){
            k--;
            if(k==0)return n;
        }
    }
    return 1;
}

int toInt(char *arr){											/// Extract int from char array
    int r=0;
    while( (*arr<'0' and *arr>'9'))arr++;
    for (; arr ; arr++){
        if(*arr<'0' or *arr>'9')break;
        r=r*10+ (*arr-'0');
    }
    return r;
}

int copyInt(uint32_t number,char* a){							/// copy int to char array
	int max=10,i;
    for(i=max-1; i>=0 ; i--){
        a[i]=hex2char(number%10);
        number/=10;
    }
    for (int i = 0; i <max-1 ; ++i)
    {
    	if (a[i]=='0')
    	{
    		a[i]=' ';
    	}else{
    		break;
    	}
    }
	a[max]='\0';
}

void shell_step(shellstate_t& state){
    state.changed=0;
    //
    //one way:
    // if a function is enabled in state
    //   call that function( with arguments stored in state) ;
    //
    bool valueInCalc=0;
    if (state.f_call==2 and state.f_done)							/// COROUTINE DONE
    {
        // is coroutine done or not?
        state.f_call=0;
        state.calc=state.f_ret;
        state.changed=valueInCalc=1;
    }else if (state.fibr_call==2 and state.fibr_done)				/// FIBER DONE
    {
        //else Is fiber done or not?
        state.fibr_call=0;
        state.calc=state.fibr_ret;
        state.changed=valueInCalc=1;
    }else{
        for (int i = 1; i < state.lmt; ++i){						/// FETCH OUTPUT FOR FIBER SCHEDULAR
            if (state.fs_call[i]==2 and state.fs_done[i]){
                state.fs_call[i]=0;
                state.pcount--;
                state.slotfr[state.p2s[i]]=1;
                state.changed=1;
                copyInt(state.fs_ret[i],state.fs_charo[state.p2s[i]] );
                break;
            }
        }
    }
    if (state.cstate==2)											/// DEALING WITH FUNCTION CALLS
    {
    	if (state.option>1 and state.option<=4)
    	{
    		state.changed=valueInCalc=1;
    	}
	    switch(state.option){
			case 1: memcpy(state.out,state.inp,sizeof(state.inp)); state.changed=1; break;
			case 2: state.calc=fact(toInt(state.inp)); break;
			case 3: state.calc=fib(toInt(state.inp)); break;
			case 4: state.calc=kthprime(toInt(state.inp)); break;
			case 5:	state.f_inp=toInt(state.inp);
					state.f_call=1;
	        		state.f_done=0;	
	        		break;
	        case 6:	state.fibr_inp=toInt(state.inp);
		            state.fibr_call=1;
		            state.fibr_done=0;
		            break;
		    case 7:
		    case 8:	state.fsc_i=toInt(state.fs_chari[state.currfs]);
		            if(state.pcount>=state.lmt){
                        state.pcount--;
		                strcpy(state.out,"No more resources left");				/// CHECKING RESOURCES LIMIT
		            }else{
		                // state.pcount++;
		                state.fsc_c=state.option-6;
		            }
		    default:;
    	}
        state.cstate=0;
    }
    if (valueInCalc){
        copyInt(state.calc,state.out);
    }
    //
    //one way:
    // if a function is enabled in stateinout
    //   call that function( with arguments stored in stateinout) ;
    //
}

//
// shellstate --> renderstate
//
void shell_render(const shellstate_t& shell, renderstate_t& render){			/// COPY REQUIRED VARIABLES TO RENDER
    render.kcount=shell.kcount; 
    render.option=shell.option;
    render.i1=shell.i1;
    render.cstate=shell.cstate;
    // render.calc=shell.calc;
    render.changed=shell.changed;
    render.pcount=shell.pcount;
    render.currfs=shell.currfs;
    memcpy(render.inp,shell.inp,sizeof(shell.inp));
    memcpy(render.out,shell.out,sizeof(shell.out));
    memcpy(render.fs_len,shell.fs_len,sizeof(shell.fs_len));
    memcpy(render.fs_call,shell.fs_call,sizeof(shell.fs_call));
    memcpy(render.p2s,shell.p2s,sizeof(shell.p2s));

    memcpy(render.fs_chari[1],shell.fs_chari[1],sizeof(shell.fs_chari[1]));  memcpy(render.fs_charo[1],shell.fs_charo[1],sizeof(shell.fs_charo[1]));
    memcpy(render.fs_chari[2],shell.fs_chari[2],sizeof(shell.fs_chari[1]));  memcpy(render.fs_charo[2],shell.fs_charo[2],sizeof(shell.fs_charo[2]));
    memcpy(render.fs_chari[3],shell.fs_chari[3],sizeof(shell.fs_chari[1]));  memcpy(render.fs_charo[3],shell.fs_charo[3],sizeof(shell.fs_charo[3]));
    memcpy(render.fs_chari[4],shell.fs_chari[4],sizeof(shell.fs_chari[1]));  memcpy(render.fs_charo[4],shell.fs_charo[4],sizeof(shell.fs_charo[4]));
    memcpy(render.fs_chari[5],shell.fs_chari[5],sizeof(shell.fs_chari[1]));  memcpy(render.fs_charo[5],shell.fs_charo[5],sizeof(shell.fs_charo[5]));
    //
    // renderstate. number of keys pressed = shellstate. number of keys pressed
    //
    // renderstate. menu highlighted = shellstate. menu highlighted
    //
    // renderstate. function result = shellstate. output argument
    //
    // etc.
    //
}


//
// compare a and b
//
bool render_eq(const renderstate_t& a, const renderstate_t& b){
    return (a.kcount==b.kcount) and (!b.changed);
}


static void fillrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawtext(int x,int y, const char* str, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawnumberinhex(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawnumberindec(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawnumberindec0(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);

//
// Given a render state, we need to write it into vgatext buffer
//

void render(const renderstate_t& state, int w, int h, addr_t vgatext_base){


    // this is just an example:
    //
    // Please create your own user interface
    //
    // You may also have simple command line user interface
    // or menu based interface or a combination of both.
    //
    int lbg=(state.cstate==0?3:3),lfg=(state.cstate==0?7:7);
    int rbg=(state.cstate==1?3:3),rfg=(state.cstate==1?7:7);
    // fillrect(0,0,w,h,3,7,      w,h,vgatext_base);
    
    int WI= 15;
    fillrect(0,0,WI,h, lbg ,lfg,  w,h,vgatext_base);
    drawrect(0,0,WI,h, 3 ,11,  w,h,vgatext_base);
    fillrect(WI,0,w,h, rbg ,rfg,  w,h,vgatext_base);
    drawrect(WI,0,w,h, 3 ,11,  w,h,vgatext_base);
    
    // fillrect(4,3,w-4,h-3,3,7,  w,h,vgatext_base);
    // drawrect(4,3,w-4,h-3,3,11,  w,h,vgatext_base);
    drawtext( (w+WI-23)/2 ,0," Keys Count: ", 13 , rbg,11,w,h,vgatext_base);
        drawnumberindec( (w+WI+3)/2 ,0,state.kcount, 11 , rbg,11,w,h,vgatext_base);
    drawtext((WI-6)/2, 0," MENU ", 6, lbg , 11,w,h,vgatext_base);
    int a[][2]={{lbg,10},{lbg,10},{lbg,10},{lbg,10},{lbg,10},{lbg,10},{lbg,10},{lbg,10},{lbg,10} } ;
    int op=state.option;
    if(state.cstate!=1){ a[op][1]=1;}
    // if(state.bion)fillrect(20,ht[op],30,ht[op]+1,4,7,  w,h,vgatext_base);

    // drawtext(1, 3,"0. ___      ", 13, a[0][0],a[0][1],w,h,vgatext_base);			/// DIAPLAYING COMMANDS
    drawtext(1, 4,"1. echo     ", 13, a[1][0],a[1][1],w,h,vgatext_base);
    drawtext(1, 5,"2. fact     ", 13, a[2][0],a[2][1],w,h,vgatext_base);
    drawtext(1, 6,"3. fib      ", 13, a[3][0],a[3][1],w,h,vgatext_base);
    drawtext(1, 7,"4. kthprime ", 13, a[4][0],a[4][1],w,h,vgatext_base);
    drawtext(1, 8,"5. kthp_coro", 13, a[5][0],a[5][1],w,h,vgatext_base);
    drawtext(1, 9,"6. kthp_fibr", 13, a[6][0],a[6][1],w,h,vgatext_base);
    drawtext(1,10,"7. totient_l", 13, a[7][0],a[7][1],w,h,vgatext_base);
    drawtext(1,11,"8. kthcomp_l", 13, a[8][0],a[8][1],w,h,vgatext_base);
    
    drawtext(WI/2-5,h-1," Running:", 13,rbg,11,w,h,vgatext_base);
        drawnumberindec0( WI/2+4 ,h-1,state.pcount, 11 , rbg,11,w,h,vgatext_base);
    
    if(state.cstate==1){
    	if (state.option<=6)
    	{
	        drawtext(WI+6,5, state.inp , w-20 , rbg , 10 ,w,h,vgatext_base);
	        drawtext(WI+6+state.i1 ,5, "_" , w-20 , rbg , 10 ,w,h,vgatext_base);
    	}else{
    		drawtext(WI+10,11+state.currfs, state.fs_chari[state.currfs] , w-20 , rbg , 10 ,w,h,vgatext_base);
	        drawtext(WI+10+state.fs_len[state.currfs] ,11+state.currfs, "_" , w-20 , rbg , 10 ,w,h,vgatext_base);
    	}
	    drawtext(WI+2,12,"f_inp1:", 14 , rbg , (state.currfs==1?1:10) ,w,h,vgatext_base);
	    drawtext(WI+2,13,"f_inp2:", 14 , rbg , (state.currfs==2?1:10) ,w,h,vgatext_base);
	    drawtext(WI+2,14,"f_inp3:", 14 , rbg , (state.currfs==3?1:10) ,w,h,vgatext_base);
	    drawtext(WI+2,15,"f_inp4:", 14 , rbg , (state.currfs==4?1:10) ,w,h,vgatext_base);
	    drawtext(WI+2,16,"f_inp5:", 14 , rbg , (state.currfs==5?1:10) ,w,h,vgatext_base);
    }else{
    	drawtext(WI+2,12,"f_inp1:", 14 , rbg , 10 ,w,h,vgatext_base);
	    drawtext(WI+2,13,"f_inp2:", 14 , rbg , 10 ,w,h,vgatext_base);
	    drawtext(WI+2,14,"f_inp3:", 14 , rbg , 10 ,w,h,vgatext_base);
	    drawtext(WI+2,15,"f_inp4:", 14 , rbg , 10 ,w,h,vgatext_base);
	    drawtext(WI+2,16,"f_inp5:", 14 , rbg , 10 ,w,h,vgatext_base);
    }
    drawtext(WI+2,4,"Enter Input:", 14 , rbg , (state.cstate==1 and state.currfs==0)?1:10 ,w,h,vgatext_base);

    drawtext(WI+2,7,"Last Output:", w-20, rbg ,10,w,h,vgatext_base);
    drawtext(WI+6,8, state.out , w-20, rbg,10,w,h,vgatext_base);


    /// fIBER OUTPUTS
    drawtext(40,12,"L1:", 14, rbg,10,w,h,vgatext_base); drawtext(43,12, state.fs_charo[1] , w, rbg,10,w,h,vgatext_base);
    drawtext(40,13,"L2:", 14, rbg,10,w,h,vgatext_base); drawtext(43,13, state.fs_charo[2] , w, rbg,10,w,h,vgatext_base);
    drawtext(40,14,"L3:", 14, rbg,10,w,h,vgatext_base); drawtext(43,14, state.fs_charo[3] , w, rbg,10,w,h,vgatext_base);
    drawtext(40,15,"L4:", 14, rbg,10,w,h,vgatext_base); drawtext(43,15, state.fs_charo[4] , w, rbg,10,w,h,vgatext_base);
    drawtext(40,16,"L5:", 14, rbg,10,w,h,vgatext_base); drawtext(43,16, state.fs_charo[5] , w, rbg,10,w,h,vgatext_base);
}


//
//
// helper functions
//
//

static void writecharxy(int x, int y, uint8_t c, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  vgatext::writechar(y*w+x,c,bg,fg,vgatext_base);
}

static void fillrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  for(int y=y0;y<y1;y++){
    for(int x=x0;x<x1;x++){
      writecharxy(x,y,0,bg,fg,w,h,vgatext_base);
    }
  }
}

static void drawrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){

  writecharxy(x0,  y0,  0xc9, bg,fg, w,h,vgatext_base);
  writecharxy(x1-1,y0,  0xbb, bg,fg, w,h,vgatext_base);
  writecharxy(x0,  y1-1,0xc8, bg,fg, w,h,vgatext_base);
  writecharxy(x1-1,y1-1,0xbc, bg,fg, w,h,vgatext_base);

  for(int x=x0+1; x+1 < x1; x++){
    writecharxy(x,y0, 0xcd, bg,fg, w,h,vgatext_base);
  }

  for(int x=x0+1; x+1 < x1; x++){
    writecharxy(x,y1-1, 0xcd, bg,fg, w,h,vgatext_base);
  }

  for(int y=y0+1; y+1 < y1; y++){
    writecharxy(x0,y, 0xba, bg,fg, w,h,vgatext_base);
  }

  for(int y=y0+1; y+1 < y1; y++){
    writecharxy(x1-1,y, 0xba, bg,fg, w,h,vgatext_base);
  }
}

static void drawtext(int x,int y, const char* str, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  for(int i=0;i<maxw;i++){
    writecharxy(x+i,y,str[i],bg,fg,w,h,vgatext_base);
    if(!str[i]){
      break;
    }
  }
}

static void drawnumberinhex(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  enum {max=sizeof(uint32_t)*2+1};
  char a[max];
  for(int i=0;i<max-1;i++){
    a[max-1-i-1]=hex2char(number%16);
    number=number/16;
  }
  a[max-1]='\0';

  drawtext(x,y,a,maxw,bg,fg,w,h,vgatext_base);
}

static void drawnumberindec(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  enum {max=sizeof(uint32_t)*2+2};
  char a[max];
  for(int i=0;i<max-1;i++){
    a[max-1-i-1]=hex2char(number%10);
    number=number/10;
  }
  a[max-1]='\0';

  drawtext(x,y,a,maxw,bg,fg,w,h,vgatext_base);
}

static void drawnumberindec0(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  enum {max=sizeof(uint32_t)*2+2};
  char a[max];
  for(int i=0;i<max-1;i++){
    a[max-1-i-1]=hex2char(number%10);
    number=number/10;
  }
  a[max-1]='\0';
  int i;
  for (i = 0; i < max-2; ++i)
  {
  	if (a[i]=='0');
  	else break;
  }
  drawtext(x,y,a+i,maxw,bg,fg,w,h,vgatext_base);
}