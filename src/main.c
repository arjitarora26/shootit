#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "hoop.h"
#include "ball.h"
#include "field.h"
#include "numbers.h"
#include "light.h"

/* timer macros */
#define TIMER_BALL 0
#define TIMER_TIME 1
#define TIMER_STRENGTH 2


/* flags */
static int scoreFlag;
static int lightSwitch;
static int animation_active;
static int hardModeFlag;

/* data */
static float rotation_speed;
static float timePassed;
static float angle;
static float ball_strength;
static float lookAngle;
static time_t initial_time;
static int score;


/* Callback functions */ 
static void on_keyboard(unsigned char key, int mouse_x,int mouse_y);
static void on_reshape(int width, int height);
static void on_timer(int value);
static void on_timer_time(int value);
static void on_timer_strength(int value);
static void on_display(void);

/* initialize and restart functions */
static void init(void);
static void restart(void);


int main(int argc , char **argv){
    
    /* Set up window and libraries */
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutInitWindowPosition(0,0);
    glutCreateWindow("ShootIT");
    
    /* Initialize everyting */
    init();

    /* Call callback fucntions and start the clock */
    glutDisplayFunc(on_display);
    glutReshapeFunc(on_reshape);
    glutTimerFunc(30,on_timer_time,TIMER_TIME);
    glutKeyboardFunc(on_keyboard);
    
    glClearColor(1,1,1,0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glutMainLoop();

    
    return 0;
}

static void on_reshape(int width, int height){
    
    /* Set up viewPort,projection matrix,itentity matrix for multiplication, pyramid */
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60,(float)width/height, 1, 1200);
    
}

static void on_display(void){
    
    /* Clear buffers*/ 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLineWidth(5);

    /* Initialize Modlview and LookAt */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* Seting up light and lightSwitch for light chages */
    headLightOn(lightSwitch);
    gluLookAt(100*cos(lookAngle),0,100*sin(lookAngle),0,0,0,0,1,0);
    setUpLight(lightSwitch);
    
    
    /* Drawing objects */
    drawSemaphore(initial_time,5);
    draw_field();
    draw_ball(x_curr , y_curr ,rotation_speed);
    draw_hoop();
    drawScore(score);    
    if(!animation_active && !hardModeFlag){
        drawShootAssistance(ball_strength,angle);
    }


    glutSwapBuffers();
    


    
}

/* Timer for throwing the ball */
static void on_timer(int value){
    if(value!=0){
        return;
    }
    
    /* Update coordinates of ball */
    rotation_speed += 30;
    timePassed += 0.65;
    updateBallPosition(timePassed,angle,ball_strength);
    
    
    /* Collision with the rim */
    if(x_curr >= -50 && x_curr <= -40 && y_curr <= 15 && y_curr >= 8){
        angle = 90;
        scoreFlag = 1;
    } 

    /* Collision with the backboard */
    if(checkBackboardCollision()){
       setBackboardFlag(1);
    }
   
    /* Collision with the floor */
    if(checkFloorCollision()){
        restart();
        if(scoreFlag && timeUp){
            score++;
            scoreFlag = 0;
        }
    }
    
    glutPostRedisplay();
    if(animation_active){
        glutTimerFunc(60,on_timer,TIMER_BALL);
    }
    
    
}

/* Timer for the time semaphore */
static void on_timer_time(int value){
    if(value != 1)
        return;
    if(timeUp){
        glutPostRedisplay();
        glutTimerFunc(60,on_timer_time,TIMER_TIME);
    }
}

/* Timer for shadow shoot / shoot assistance */
static void on_timer_strength(int value){
    if(value != 3)
        return;
    if(!animation_active){
        glutPostRedisplay();
        glutTimerFunc(60,on_timer_strength,TIMER_STRENGTH);
    }
}
/* Intialize everyting */
static void init(){
    score = 0;
    scoreFlag =0;
    lookAngle = 90 * PI/180;
    lightSwitch = 0;
    hardModeFlag = 0;
    initial_time = time(NULL);
    srand(time(NULL));    
    restart();
}
/* Generate new ball */
static void restart(){
    animation_active = 0;
    timePassed = 0;
    angle = 45;
    setBackboardFlag(0);
    ball_strength = 1;
    initBallPosition();
}


/* Keyboard actions*/
static void on_keyboard(unsigned char key, int mouse_x, int mouse_y ){
    
    // printf("Key pressed : %c\n", key)s;

    switch(key){
        /* click b/B to shoot */
        case 'b':
        case 'B':
            if(!animation_active && timeUp){
                animation_active = 1;
                glutTimerFunc(60,on_timer,TIMER_BALL);
            }
            break;
        /* click w/W to change the angle of the shot */
        case 'w':
        case 'W':
            if(!animation_active){
                // printf("Hit w\n");
                angle+=3;
                if(angle > 89){
                    angle = 89;
                }
            }
            break;
        /* click s/S to change the angle of the shot */
        case 's':
        case 'S':
            if(!animation_active){
                
                angle-=3;
                if(angle < -89){
                    angle = -89;
                }
            }
            break;
        /* click a/A to rotate the camera around the field */
        case 'a':
        case 'A':
            lookAngle += (5 * PI/180);
            glutPostRedisplay();
            break;
        /* click d/D to rotate the camera around the field */
        case 'd':
        case 'D':
            lookAngle -= (5 * PI/180);
            glutPostRedisplay();
            break;
        /* click l/L to turn on/off the headlight */
        case 'l':
        case 'L':
            lightSwitch = lightSwitch? 0 : 1;  
            glutPostRedisplay();
            break;
        /* click j/J to change the strength of the shot */
        case 'j':
        case 'J':
            if(!animation_active && timeUp){
                ball_strength += 0.015;
                if(ball_strength > 1.5){
                    ball_strength = 1.5;
                }
                glutTimerFunc(60,on_timer_strength,TIMER_STRENGTH);
            }
            break;
        /* click k/K to change the strength of the shot */
        case 'k':
        case 'K':
             if(!animation_active && timeUp){
                ball_strength -= 0.015;
                
                if(ball_strength < 0.5){
                    ball_strength = 0.5;
                }
                
                glutTimerFunc(60,on_timer_strength,TIMER_STRENGTH);
            }
            break;
        /* click h/H to turn on hardMode - no shot assistance */
        case 'h':
        case 'H':
            hardModeFlag = !hardModeFlag;
            break;
        /* click Esc to exit */
        case 27:
            exit(0);
            break;
        
    }
    
}




