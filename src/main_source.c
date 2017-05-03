/**
 * PUCRS - FACIN
 * Computação Gráfica II - Trabalho I
 *
 * teclas:
 * move objeto para a esquerda: q
 * move objeto para a direita: w
 * move objeto para frente: a
 * move objeto para trás: s
 * move objeto para cima: z
 * move objeto para baixo: x
 * incluir objeto: i
 * excluir objeto: d
 * seleciona objeto anterior: e
 * seleciona próximo objeto: r
 */

#include <AR/arMulti.h>
#include <AR/config.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/gsub.h>
#include <AR/ar.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <windows.h>
#include <VideoIM.h>
#include <stdio.h>
#include <stdlib.h>
#define MAXIMO_OBJETOS 20

//estruturas
typedef struct
{
    double x, y, z;
} ponto_3d;
typedef struct
{
    ponto_3d posicao, rotacao, escala;
    int id, tipo;
} objeto_grafico;

//variáveis globais
char *vconf = "Data\\WDM_camera_flipV.xml";
char *cparam_name = "Data/camera_para.dat";
char *config_name = "Data/multi/marker.dat";
ARMultiMarkerInfoT *config;
ARParam cparam;
int xsize, ysize;
int thresh = 100;
int count = 0;
int gerador_id = 0;
int contador_objetos = 0;
int objeto_selecionado = 0;
objeto_grafico lista_objetos[MAXIMO_OBJETOS];

//protótipos de funções
static void init(void);
static void cleanup(void);
static void keyEvent( unsigned char key, int x, int y);
static void mainLoop(void);
static void draw(double trans1[3][4], double trans2[3][4], int posicao_do_objeto_na_lista);
void adiciona_objeto(objeto_grafico novo_objeto);
void remove_objeto(int id_objeto);

/**
 * adiciona um novo objeto na lista de objetos
 */
void adiciona_objeto(objeto_grafico novo_objeto)
{
    if(contador_objetos < MAXIMO_OBJETOS)
    {
        lista_objetos[contador_objetos] = novo_objeto;
        contador_objetos++;
    }
}

/**
 * remove um objeto da lista de objetos
 */
void remove_objeto(int id_objeto)
{
    int i,j;

    if(contador_objetos > 0)
    {
        for(i = 0; i < contador_objetos; i++)
        {
            if(id_objeto == lista_objetos[i].id)
            {
                for(j = i; j < contador_objetos-1; j++)
                {
                    lista_objetos[j] = lista_objetos[j+1];
                }
                contador_objetos--;
            }
        }
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    init();

    arVideoCapStart();
    argMainLoop(NULL,keyEvent,mainLoop);
    return (0);
}

/**
 * main loop
 */
static void mainLoop(void)
{
    glLoadIdentity();

    ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    double          err;
    int             i;

    //grab a vide frame
    if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL )
    {
        arUtilSleep(2);
        return;
    }
    if( count == 0 ) arUtilTimerReset();
    count++;

    //detect the markers in the video frame
    if( arDetectMarkerLite(dataPtr, thresh, &marker_info, &marker_num) < 0 )
    {
        cleanup();
        exit(0);
    }

    argDrawMode2D();
    if( !arDebug )
    {
        argDispImage( dataPtr, 0,0 );
    }
    else
    {
        argDispImage( dataPtr, 1, 1 );
        if( arImageProcMode == AR_IMAGE_PROC_IN_HALF )
        {
            argDispHalfImage( arImage, 0, 0 );
        }
        else
        {
            argDispImage( arImage, 0, 0);
        }

        glColor3f( 1.0, 0.0, 0.0 );
        glLineWidth( 1.0 );
        for( i = 0; i < marker_num; i++ )
        {
            argDrawSquare( marker_info[i].vertex, 0, 0 );
        }
        glLineWidth( 1.0 );
    }

    arVideoCapNext();

    if( (err=arMultiGetTransMat(marker_info, marker_num, config)) < 0 )
    {
        argSwapBuffers();
        return;
    }
    //printf("err = %f\n", err);
    if(err > 100.0 )
    {
        argSwapBuffers();
        return;
    }

    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);

    //passa por todos os marcadores
    for( i = 0; i < config->marker_num; i++ )
    {
        //se o marcador i foi capturado no frame
        if(config->marker[i].visible >= 0)
        {
            int posicao_objeto;

            if(i==0){printf("RECONHECEU A!!!\n");}
            if(i==6){printf("RECONHECEU KANDI!!!\n");}
            //desenha todos os objetos de maneira relativa ao marcador i
            for(posicao_objeto = 0; posicao_objeto < contador_objetos; posicao_objeto++)
            {
                glPushMatrix();

                draw(config->trans, config->marker[i].trans, posicao_objeto);

                glPopMatrix();
            }
        }
    }

    argSwapBuffers();
}

static void init( void )
{
    ARParam  wparam;

    /* open the video path */
    if( arVideoOpen( vconf ) < 0 ) exit(0);
    /* find the size of the window */
    if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    /* set the initial camera parameters */
    if( arParamLoad(cparam_name, 1, &wparam) < 0 )
    {
        printf("Camera parameter load error !!\n");
        exit(0);
    }
    arParamChangeSize( &wparam, xsize, ysize, &cparam );
    arInitCparam( &cparam );
    printf("*** Camera Parameter ***\n");
    arParamDisp( &cparam );

    if( (config = arMultiReadConfigFile(config_name)) == NULL )
    {
        printf("config data load error !!\n");
        exit(0);
    }

    /* open the graphics window */
    argInit( &cparam, 1.0, 0, 2, 1, 0 );
    arFittingMode   = AR_FITTING_TO_IDEAL;
    arImageProcMode = AR_IMAGE_PROC_IN_HALF;
    argDrawMode     = AR_DRAW_BY_TEXTURE_MAPPING;
    argTexmapMode   = AR_DRAW_TEXTURE_HALF_IMAGE;
}

/* cleanup function called when program exits */
static void cleanup(void)
{
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}

static void draw( double trans1[3][4], double trans2[3][4], int posicao_do_objeto_na_lista)
{
    //faz leitura das coordenadas do objeto
    double x_objeto = lista_objetos[posicao_do_objeto_na_lista].posicao.x;
    double y_objeto = lista_objetos[posicao_do_objeto_na_lista].posicao.y;
    double z_objeto = lista_objetos[posicao_do_objeto_na_lista].posicao.z;

    //faz leitura das coordenadas do marcador
    double x_marcador = trans2[0][3];
    double y_marcador = trans2[1][3];
    double z_marcador = trans2[2][3];

    //calcula a coordenada do objeto em relação ao marcador
    double x_relativo = x_objeto - x_marcador;
    double y_relativo = y_objeto - y_marcador;
    double z_relativo = z_objeto - z_marcador;

    double    gl_para[16];
    GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_ambient1[]    = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash1[]      = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   mat_flash_shiny1[]= {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //load the camera transformation matrix
    glMatrixMode(GL_MODELVIEW);
    argConvGlpara(trans1, gl_para);
    glLoadMatrixd( gl_para );
    argConvGlpara(trans2, gl_para);
    glMultMatrixd( gl_para );

    //a cor do objeto selecionado pelo usuário é azul
    if(posicao_do_objeto_na_lista == objeto_selecionado)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    }
    else
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash1);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny1);
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient1);
    }
    glMatrixMode(GL_MODELVIEW);;

    //aplica a translação relativa entre marcador e objeto
    glTranslatef( x_relativo, y_relativo, z_relativo );

    //DESENHA
    if( !arDebug ) glutSolidCube(50.0);
    else          glutWireCube(50.0);

    glDisable( GL_LIGHTING );

    glDisable( GL_DEPTH_TEST );
}

static void keyEvent( unsigned char tecla, int x, int y)
{
    //ESC => encerra programa
    if (tecla == 0x1b)
    {
        cleanup();
        exit(0);
    }
    //'d' => remove objeto selecionado
    else if (tecla=='d')
    {
        int id_do_objeto_a_ser_removido = lista_objetos[objeto_selecionado].id;
        remove_objeto(id_do_objeto_a_ser_removido);
    }
    //'i' => adiciona um objeto na lista de objetos
    else if (tecla=='i' || tecla=='I')
    {
        printf("TECLA I FOI PRESSIONADA!!!\n");
        if(contador_objetos < MAXIMO_OBJETOS)
        {
            float x,y,z;

            objeto_grafico objeto_aux;
            ponto_3d ponto_aux;
            ponto_aux.x = 0.0;
            ponto_aux.y = 0.0;
            ponto_aux.z = 0.0;
            objeto_aux.escala = ponto_aux;
            objeto_aux.rotacao = ponto_aux;
            objeto_aux.posicao = ponto_aux;
            //faz leitura da posicao desejada
            scanf("%f",&x);
            scanf("%f",&y);
            scanf("%f",&z);
            printf("FLOAT ESCRITO %f\n",x);
            objeto_aux.posicao.x = x;
            objeto_aux.posicao.y = y;
            objeto_aux.posicao.z = z;
            gerador_id++;
            objeto_aux.id = gerador_id;
            objeto_aux.tipo = 0;
            adiciona_objeto(objeto_aux);

            //objeto_selecionado = contador_objetos;
        }
    }
    //translação para a esquerda
    else if (tecla=='q')
    {
        double novo_valor = lista_objetos[objeto_selecionado].posicao.x;
        novo_valor--;
        lista_objetos[objeto_selecionado].posicao.x = novo_valor;
    }
    //translação para a direita
    else if (tecla=='w')
    {
        double novo_valor = lista_objetos[objeto_selecionado].posicao.x;
        novo_valor++;
        lista_objetos[objeto_selecionado].posicao.x = novo_valor;
    }
    //translação para frente
    else if (tecla=='a')
    {
        double novo_valor = lista_objetos[objeto_selecionado].posicao.y;
        novo_valor++;
        lista_objetos[objeto_selecionado].posicao.y = novo_valor;
    }
    //translação para trás
    else if (tecla=='s')
    {
        double novo_valor = lista_objetos[objeto_selecionado].posicao.y;
        novo_valor--;
        lista_objetos[objeto_selecionado].posicao.y = novo_valor;
    }
    //translação para cima
    else if (tecla=='z')
    {
        double novo_valor = lista_objetos[objeto_selecionado].posicao.z;
        novo_valor++;
        lista_objetos[objeto_selecionado].posicao.z = novo_valor;
    }
    //translação para baixo
    else if (tecla=='x')
    {
        double novo_valor = lista_objetos[objeto_selecionado].posicao.z;
        novo_valor--;
        lista_objetos[objeto_selecionado].posicao.z = novo_valor;
    }
    //seleciona objeto anterior
    else if(tecla=='e')
    {
        if(objeto_selecionado > 0)
        {
            objeto_selecionado--;
        }
    }
    //seleciona próximo objeto
    else if(tecla=='r')
    {
        if(objeto_selecionado < contador_objetos-1)
        {
            objeto_selecionado++;
        }
    }
}
