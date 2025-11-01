#include <GL/freeglut.h>
#include <stdlib.h>
void initialize(){
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
}
void drawImage(  ){
    glClear( GL_COLOR_BUFFER_BIT );
    glColor3f( 0.75, 0.75, 0.75 );
    glLoadIdentity();
    glBegin( GL_POLYGON );
        glVertex3f( -0.75, -0.75, -5.0 );
        glVertex3f(  0.75, -0.75, -5.0 );
        glVertex3f(  0.75,  0.75, -5.0 );
        glVertex3f( -0.75,  0.75, -5.0 );
    glEnd();
    glFlush();
}
void reshapeWindow( int width, int height ){
    glViewport( 0, 0, (GLsizei) width, (GLsizei) height );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 25.0 );
    glMatrixMode( GL_MODELVIEW );
}
int main(int argc, char **argv){
    
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(300, 300);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Example");
    initialize(); 
    glutDisplayFunc( drawImage ); 
    glutReshapeFunc( reshapeWindow );
    
    glutMainLoop(); 
    return 0;
}
