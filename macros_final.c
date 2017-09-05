/*
    Implementing Keyboard Macros
    System Practicum - 2017
    Group Members - Saif Ali Akhtar
                    Purushottam
                    Priyadharshinee S.
                    Rishabh Kumar
    Project Mentor - TAG Sir
*/

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Struct to Hold Keyboard Macros
typedef struct {
    KeySym key;
    KeySym exp[128];
    int len;
} macros;

// Check whether a particular key is present in the list of available macros
int check(macros A[],int size,int rkey) {
    int i;
    for(i = 0; i < size; i++) {
        if(A[i].key == rkey)
            return i;
    }
    return -1;
}

/* 
    Sends a particular keycode to a given active X11 Display 
    Source : https://bharathisubramanian.wordpress.com/2010/03/14/x11-fake-key-event-generation-using-xtest-ext/
*/
void SendKey (Display * disp, KeySym keysym, KeySym modsym){
    KeyCode keycode = 0, modcode = 0;
    keycode = XKeysymToKeycode (disp, keysym);
    if (keycode == 0) return;
    XTestGrabControl (disp, True);
    if (modsym != 0) {
    modcode = XKeysymToKeycode(disp, modsym);
    XTestFakeKeyEvent (disp, modcode, True, 0);
    }
    XTestFakeKeyEvent (disp, keycode, True, 0);
    XTestFakeKeyEvent (disp, keycode, False, 0); 
    if (modsym != 0)
    XTestFakeKeyEvent (disp, modcode, False, 0);
    XSync (disp, False);
    XTestGrabControl (disp, False);
}

// Prints a particular macro type element 
void printM(macros M) {
    int i,k,l;
    char *str;
    for(i = 0; i < M.len; i++) {
        k = M.exp[2*i];
        l = M.exp[2*i+1];
        if(k==65293) {
            printf("<Enter>");
        }
        else if(k==65289) {
            printf("<TAB>");
        }
        else {
            str=XKeysymToString(l);
            if(l!=0) printf("[<%s>+%c]",str,k);
            else printf("%c",k);
        }
        //SendKey(disp,k,l);
    }
}

// Writes the available macros to a file
void CreateKeyFile(macros A[], int size) {
    FILE *fp = fopen(".keys","w");
    int i,j;
    fprintf(fp,"%d\n",size);
    for(i = 0; i < size; i++) {
        fprintf(fp,"%lu %d ",A[i].key,A[i].len);
        for(j = 0; j < 2*A[i].len; j++) {
            fprintf(fp,"%lu ",A[i].exp[j]);
        }
        fprintf(fp, "\n" );
    }
    fclose(fp);
}

// Fetches the available macros from a given file
int ReadMacrosFile(macros M[]) {
    int i,j,totalP;
    FILE *fp = fopen(".keys","r");
    fscanf(fp,"%d",&totalP);
    for(i = 0; i < totalP; i++) {
        fscanf(fp,"%lu %d",&M[i].key,&M[i].len);
        for(j = 0; j < 2*M[i].len; j++) {
            fscanf(fp,"%lu",&M[i].exp[j]);
        }
    }
    fclose(fp);
    return totalP;
}

int main()
{
    int i,j,k,l;
    int totalP;
    int op,s;
    int allowed[] = {41,33,64,35,36,37,94,38,42,40};
    int modify = -1;
    macros M[10];
    Display *display,*disp;
    Window window;
    XEvent event;
    KeySym mykey;
    if(access(".keys", F_OK) == -1) {
        FILE *temp_fp = fopen(".keys", "w+");
        fprintf(temp_fp, "0\n");
        fclose(temp_fp);
    }
    totalP = ReadMacrosFile(M);
    //printf("%d\n",totalP );
    printf("Welcome to Keyboard Macros Program.\n");
    printf("1. Record a macro\t2. Modify macros\n3. Execute macros\t4. Exit\n");
    while(1) {
        modify = -1;
        printf("Enter your input : ");
        scanf("%d",&op);
        if(op == 4) {
            break;
        }
        else if(op == 3) {
            printf("In order to execute the macro in any application first select the\napplication then select the X11 Display and press your macro shortcut.\n");
            printf("Press 'Esc' to quit the X11 display gracefully.\nPlease ensure that CAPS LOCK is OFF\n");
            totalP = ReadMacrosFile(M);
            //printf("Here\n");
            display = XOpenDisplay(NULL);
            if (display == NULL) {
                printf("Cannot open display\n");
                exit(1);
            }
            /*
                Refrences used for capturing keystrokes
                1) http://stackoverflow.com/questions/11315001/x-keypress-release-events-capturing-irrespective-of-window-in-focus
                2) https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html
                3) https://gist.github.com/javiercantero/7753445
            */
            s = DefaultScreen(display);
            window = XCreateSimpleWindow(display, RootWindow(display, s), 10, 10, 200, 200, 1,BlackPixel(display, s), WhitePixel(display, s));
            XSelectInput(display, window, KeyPressMask | KeyReleaseMask );
            XMapWindow(display, window);
            int ctrl = 0,shift = 0;
            int tries = 0;
            while (1) {
                XNextEvent(display, &event);
                if (event.type == KeyPress) {
                    mykey = XkbKeycodeToKeysym( display, event.xkey.keycode, 0, event.xkey.state & ShiftMask ? 1 : 0);
                    if(mykey == 65507) {
                        //printf("Here!\n");
                        ctrl = 1;
                    }
                    else if(mykey == 65505) {
                        //printf("Here2!\n");
                        shift = 1;
                    }
                }
                if (event.type == KeyRelease) {
                    KeySym mykey = XkbKeycodeToKeysym( display, event.xkey.keycode, 0, event.xkey.state & ShiftMask ? 1 : 0);
                    if(mykey == 65507)
                        ctrl = 0;
                    else if(mykey == 65505)
                        shift = 0;
                    else {
                        j = check(M,totalP,mykey);
                        if( j != -1 && ctrl && shift) {
                            //printf("Here3!\n");
                            printf("Macro Shortcut Ctrl+Shift+%d is being executed\n",j);
                            sleep(1);
                            disp = XOpenDisplay (NULL);
                            SendKey (disp, XK_Tab, XK_Alt_L);
                            sleep(1);
                            disp = XOpenDisplay (NULL);
                            //printf("%d\n",M[j].len);
                            for(i = 0; i < M[j].len; i++) {
                                k = M[j].exp[2*i];
                                l = M[j].exp[2*i+1];
                                //printf("%d %d\n",k,l);
                                SendKey(disp,k,l);
                            }
                        }
                        else {
                            tries++;
                            if(tries == 10) {
                                tries = 0;
                                printf("Try LCtrl+LShift+<num>. Press 'Esc' to exit.\n");
                            }
                        }
                    }
                    //printf( "KP:%lu\n",mykey);
                    if ( event.xkey.keycode == 0x09 )
                        break;
                }
            }
            XCloseDisplay(display);
        }
        else if(op == 2) {
            totalP = ReadMacrosFile(M);
            printf("Modify Existing Macros\n");
            printf("S.No : Macro Shortcut : Macro Extension\n");
            for(i = 0; i < totalP; i++) {
                printf("%d : Ctrl+Shift+%d : ",i,i);
                printM(M[i]);
                printf("\n");
            }
            printf("Enter the macro you want to edit : ");
            scanf("%d",&modify);
            if(modify >= 0 && modify < totalP) {
                modify;
                goto Rec;    
            }
            else {
                printf("Wrong Index Selected\n");
            }
        }
        else if(op == 1) {
            if(totalP == 10) {
                printf("Cannot add any more macros. Please edit any existing ones.\n");
                continue;
            }
            Rec:
            printf("Press HOME to Start Recording and END to Stop Recording\n");
            printf("Maximum Macro length is 64 keys\n");
            printf("Press 'Esc' to close the X11 Display gracefully and discard the changes.\n");
            display = XOpenDisplay(NULL);
            if (display == NULL) {
                printf("Cannot open display\n");
                exit(1);
            }
            s = DefaultScreen(display);
            int record = 0;
            int flag = 0;
            unsigned long v[128];
            int len = 0;
            int ctrl = 0;
            int shift = 0;
            int caps = 0;
            window = XCreateSimpleWindow(display, RootWindow(display, s), 10, 10, 200, 200, 1,BlackPixel(display, s), WhitePixel(display, s));
            XSelectInput(display, window, KeyPressMask | KeyReleaseMask );
            XMapWindow(display, window);
            while(1) {
                XNextEvent(display, &event);
                if (event.type == KeyPress) {
                    if ( event.xkey.keycode == 0x09 ) {
                        XCloseDisplay(display);
                        break;
                    }
                    mykey = XkbKeycodeToKeysym( display, event.xkey.keycode, 0, event.xkey.state & ShiftMask ? 1 : 0);
                    if(mykey == 65360 && !record) {
                        printf("Started Recording Macro!\n");
                        record = 1;
                    }
                    else if(mykey == 65367 && record) {
                        printf("Stopped Recording Macro!\n");
                        flag = 1;
                        XCloseDisplay(display);
                        record = 0;
                        break;
                    }
                    else if((mykey == 65505 || mykey == 65506) && record)
                        shift = 1;
                    else if((mykey == 65507 || mykey == 65508) && record)
                        ctrl = 1;
                    else if(mykey == 65509  && record)
                        caps = !caps;
                }
                if( event.type == KeyRelease) {
                    mykey = XkbKeycodeToKeysym( display, event.xkey.keycode, 0, event.xkey.state & ShiftMask ? 1 : 0);
                    if(record == 1) {
                        if(len == 128) {
                            printf("Maximum Keys Recorded!\nStopped Recording Macro!\n");
                            XCloseDisplay(display);
                            record = 0;
                            flag = 1;
                            break;
                        }
                        else if(mykey == 65360) {

                        }
                        //Checking for Shift or Ctrl Modifiers
                        else if((mykey == 65505 || mykey == 65506) && len > 1) {
                            shift = 0;
                        }
                        else if((mykey == 65507 || mykey == 65508) && len > 1) {
                            ctrl = 0;
                        }
                        else if(mykey == 65509) {}
                        else {
                            v[len++] = mykey;
                            if(shift && caps)
                                v[len++] = 0;
                            else if(caps)
                                v[len++] = 65505;
                            else if(shift)
                                v[len++] = 65505;
                            else if(ctrl)
                                v[len++] = 65507;
                            else
                                v[len++] = 0;
                        }
                    }
                }
            }
            int index = (modify == -1)?totalP:modify;
            if(flag == 1) {
                M[index].key = allowed[index];
                M[index].len = len/2;
                for(i = 0; i < len/2; i++) {
                    k = v[2*i];
                    l = v[2*i+1];
                    M[index].exp[2*i] = k;
                    M[index].exp[2*i+1] = l;
                    //printf("%d %d ",k,l);
                    //SendKey(disp,k,l);
                }
                printf("Recorded Message : ");
                printM(M[index]);
                printf("\n");
                if(modify != -1) {
                    printf("Your Macro is now modified\nMacro Shortcut is Ctrl+Shift+%d\n",index);
                    CreateKeyFile(M,totalP);
                    continue;
                }
                totalP++;
                printf("Your Macro is ready to use\nMacro Shortcut is Ctrl+Shift+%d\n",totalP-1);
                CreateKeyFile(M,totalP);
            }
            else {
                printf("Esc encountered. All changes were discarded.\n");
            }
        }
        else {
            printf("Wrong Input!\n");
            break;
        }
    }
    return 0;
}