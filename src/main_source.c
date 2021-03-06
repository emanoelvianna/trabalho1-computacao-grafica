/**
 * Universidade: PUCRS
 * Unidade: FACIN
 * Curso: Bacharelado em Ciência da Computação
 * Disciplina: Computação Gráfica II
 * Turma: 128
 * Trabalho: Trbalho I
 * Alunos: Emanoel Vianna, Gabriell Araujo
 *
 * teclas:
 * w,a,s,d: move objeto selecionado
 * e: seleciona próximo objeto
 * q: seleciona objeto anterior
 * r: deleta objeto já selecionado
 * esc: encerra o programa
 *
 * mouse:
 * botão direito: abre menu
 */

#ifdef _WIN32
#include <windows.h>
#include <VideoIM.h>
#endif

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif

#include <AR/arMulti.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/gsub.h>
#include <AR/ar.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//constantes
#define MAXIMO_OBJETOS 20
#define LUGARES_NA_SALA 4
#define X_PAREDE_DA_ESQUERDA -35
#define X_PAREDE_DA_DIREITA 275
#define Y_PAREDE_DO_FUNDO 0
#define Z_CHAO 0

//estrutura ponto_3d
typedef struct
{
    double x, y, z;
} ponto_3d;

//estrutura objeto_grafico
typedef struct
{
    //parede_esqueda=0,parede_direita=1,parede_fundo=2,chao=3
    bool lugar[LUGARES_NA_SALA];
    ponto_3d posicao, rotacao, escala;
    double x_padrao, y_padrao, z_padrao;
    int id, tipo, lugar_escolhido;
} objeto_grafico;

#ifdef _WIN32
char *vconf = "Data\\WDM_camera_flipV.xml";
#else
char *vconf = "";
#endif

//variáveis globais
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
bool mostrar_paredes_e_chao = TRUE;

//posições padrões
ponto_3d posicao_default_parede_da_esquerda;
ponto_3d posicao_default_parede_da_direita;
ponto_3d posicao_default_parede_do_fundo;
ponto_3d posicao_default_chao;

//protótipos de funções
void cria_objeto(ponto_3d arg_posicao, ponto_3d arg_rotacao, ponto_3d arg_escala, int arg_tipo, int arg_lugar);
void adiciona_objeto(objeto_grafico novo_objeto);
void remove_objeto(int id_objeto);
void desenha_objeto(int tipo_do_objeto);
void desenhaCubo(float height, float width, float depth);
void desenha_mesa();
void desenha_cadeira(float x, float y, float z, float degrees);
void desenha_quadro(int arg_lugar);
void desenha_parede(int arg_lugar);
void desenha_chao();
void cria_menu(void);
void menu(int acao);
static void draw(double trans1[3][4], double trans2[3][4], int posicao_do_objeto_na_lista);
static void draw_objetos_estaticos(double trans1[3][4], double trans2[3][4]);
static void mainLoop(void);
static void init(void);
static void cleanup(void);
static void keyEvent(unsigned char key, int x, int y);

/**
 * cria um objeto e o adiciona na lista de objetos
 */
void cria_objeto(ponto_3d arg_posicao, ponto_3d arg_rotacao, ponto_3d arg_escala, int arg_tipo, int arg_lugar)
{
    objeto_grafico objeto_aux;

    objeto_aux.posicao = arg_posicao;
    objeto_aux.x_padrao = arg_posicao.x;
    objeto_aux.y_padrao = arg_posicao.y;
    objeto_aux.z_padrao = arg_posicao.z;
    objeto_aux.rotacao = arg_rotacao;
    objeto_aux.escala = arg_escala;

    gerador_id++;
    objeto_aux.id = gerador_id;

    objeto_aux.tipo = arg_tipo;

    int i;
    for (i = 0; i < LUGARES_NA_SALA; i++)
    {
        objeto_aux.lugar[i] = FALSE;
    }
    objeto_aux.lugar[arg_lugar] = TRUE;

    objeto_aux.lugar_escolhido = arg_lugar;

    adiciona_objeto(objeto_aux);
}

/**
 * adiciona um novo objeto na lista de objetos
 */
void adiciona_objeto(objeto_grafico novo_objeto)
{
    if (contador_objetos < MAXIMO_OBJETOS)
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
    int i, j;

    if (contador_objetos > 0)
    {
        for (i = 0; i < contador_objetos; i++)
        {
            if (id_objeto == lista_objetos[i].id)
            {
                for (j = i; j < contador_objetos - 1; j++)
                {
                    lista_objetos[j] = lista_objetos[j + 1];
                }
                contador_objetos--;
            }
        }
    }
}

/**
 * desenha um objeto
 */
void desenha_objeto(int posicao_lista_objetos)
{
    int tipo_objeto = lista_objetos[posicao_lista_objetos].tipo;
    int lugar = lista_objetos[posicao_lista_objetos].lugar_escolhido;

    switch(tipo_objeto)
    {
    //cadeira
    case 0:
    {
        desenha_cadeira(35.f, 0.f, 10.f, 270.f);
        break;
    }
    //mesa
    case 1:
    {
        desenha_mesa();
        break;
    }
    //quadro
    case 2:
    {
        desenha_quadro(lugar);
        break;
    }
    }
}

/**
 * desenha um cubo
 */
void desenhaCubo(float height, float width, float depth)
{

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.0f, height, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(width, height, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(width, 0.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -depth);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.0f, height, -depth);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(width, height, -depth);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(width, 0.0f, -depth);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.0f, height, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.0f, height, -depth);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(width, height, -depth);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(width, height, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0.0f, height, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.0f, height, -depth);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -depth);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(width, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(width, height, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(width, height, -depth);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(width, 0.0f, -depth);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -depth);
    glVertex3f(width, 0.0f, -depth);
    glVertex3f(width, 0.0f, 0.0f);
    glEnd();
}

/**
 * desenha uma cadeira
 */
void desenha_cadeira(float x, float y, float z, float degrees)
{
    glPushMatrix();
    glScalef(5,5,5);
    glRotatef(90,1,0,0);
    glRotatef(90,0,1,0);

    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(degrees, 0.0f, 1.0f, 0.0f);

    glPushMatrix();
    glTranslatef(5.0f, 0.0f, 20.0f);
    desenhaCubo(0.5f, 7, 7);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5.0f, -5.0f, 20.0f);
    desenhaCubo(5, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(11.0f, -5.0f, 20.0f);
    desenhaCubo(5, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(11.0f, -5.0f, 14.0f);
    desenhaCubo(5, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5.0f, -5.0f, 14.0f);
    desenhaCubo(5, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5.0f, 0.5f, 14.0f);
    desenhaCubo(6, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(11.5f, 0.5f, 14.0f);
    desenhaCubo(6, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(7.0f, 0.5f, 14.0f);
    desenhaCubo(6, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(9.5f, 0.5f, 14.0f);
    desenhaCubo(6, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5.0f, 6.5f, 14.0f);
    desenhaCubo(2, 7, 0.5f);
    glPopMatrix();

    glPopMatrix();

    glPopMatrix();
}

/**
 * desenha uma mesa
 */
void desenha_mesa()
{
    float valor_escala = 7;

    glPushMatrix();
    glScalef(valor_escala,valor_escala,valor_escala);
    glTranslatef(-10, 25, 0);
    glRotatef(90,1,0,0);

    glPushMatrix();
    glTranslatef(2.0f, 0.0f, 3.0f);

    glPushMatrix();
    glTranslatef(5.0f, 3.0f, 20.0f);
    desenhaCubo(0.5f, 10, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5.0f, -5.0f, 20.0f);
    desenhaCubo(8, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(14.0f, -5.0f, 20.0f);
    desenhaCubo(8, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(14.0f, -5.0f, 11.0f);
    desenhaCubo(8, 0.5f, 0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5.0f, -5.0f, 11.0f);
    desenhaCubo(8, 0.5f, 0.5f);
    glPopMatrix();

    glPopMatrix();

    glPopMatrix();
}

/**
 * desenha um quadro
 */
void desenha_quadro(int arg_lugar)
{
    float valor = 2.5;

    //parede da esquerda
    if(arg_lugar == 0)
    {
        glPushMatrix();
        glScalef(valor,valor,valor);
        glRotatef(90,0,0,1);

        glPushMatrix();
        glTranslatef(2.0f, 0.0f, 3.0f);

        glPushMatrix();
        glTranslatef(5.0f, 3.0f, 20.0f);
        desenhaCubo(0.5f, 10, 10);
        glPopMatrix();

        glPopMatrix();

        glPopMatrix();
    }
    //parede da direita
    else if(arg_lugar == 1)
    {
        glPushMatrix();
        glScalef(valor,valor,valor);
        glRotatef(-90,0,0,1);

        glPushMatrix();
        glTranslatef(2.0f, 0.0f, 3.0f);

        glPushMatrix();
        glTranslatef(5.0f, 3.0f, 20.0f);
        desenhaCubo(0.5f, 10, 10);
        glPopMatrix();

        glPopMatrix();

        glPopMatrix();
    }
    //parede do fundo
    else if(arg_lugar == 2)
    {
        glPushMatrix();
        glScalef(valor,valor,valor);

        glPushMatrix();
        glTranslatef(2.0f, 0.0f, 3.0f);

        glPushMatrix();
        glTranslatef(5.0f, 3.0f, 20.0f);
        desenhaCubo(0.5f, 10, 10);
        glPopMatrix();

        glPopMatrix();

        glPopMatrix();
    }
}

/**
 * desenha uma parede
 */
void desenha_parede(int arg_lugar)
{
    float color_x = 0.59;
    float color_y = 0.59;
    float color_z = 0.59;

    //parede da esquerda
    if (arg_lugar == 0)
    {
        glPushMatrix();
        glTranslatef(-20,0,30);
        glRotatef(180, 0, 0, 1);
        glColor3f(color_x, color_y, color_z);
        //ajuste conforme tamanho da caixa
        glScalef(5, 100, 70);
        glutSolidCube(3);
        glPopMatrix();
    }
    //parede da direita
    else if (arg_lugar == 1)
    {
        glPushMatrix();
        glTranslatef(50,0,30);
        glRotatef(180, 0, 0, 1);
        glColor3f(color_x, color_y, color_z);
        //ajuste conforme tamanho da caixa
        glScalef(5, 100, 70);
        glutSolidCube(3);
        glPopMatrix();
    }
    //parede do fundo
    else if (arg_lugar == 2) //fundo
    {
        int desloca_x = 150;

        //parte de cima
        glPushMatrix();
        glTranslatef(desloca_x,0,0);
        glColor3f(color_x, color_y, color_z);
        glTranslatef(2.0f, 0.0f, 10.0f);
        glRotated(90, 0, 0, 1);
        //ajuste conforme tamanho da caixa
        glScalef(5, 100.0f, 50);
        glutSolidCube(3);
        glPopMatrix();
        //parte de baixo
        glPushMatrix();
        glTranslatef(desloca_x,0,50);
        glColor3f(color_x, color_y, color_z);
        glTranslatef(2.0f, 0.0f, 90.0f);
        glRotated(90, 0, 0, 1);
        //ajuste conforme tamanho da caixa
        glScalef(5, 100.0f, 10);
        glutSolidCube(3);
        glPopMatrix();
        //parte da esquerda
        glPushMatrix();
        glTranslatef(desloca_x,0,0);
        glColor3f(color_x, color_y, color_z);
        glTranslatef(-90.0f, 0.0f, 50.0f);
        glRotated(90, 0, 0, 1);
        //ajuste conforme tamanho da caixa
        glScalef(5, 40.0f, 70);
        glutSolidCube(3);
        glPopMatrix();
        //parte da direita
        glPushMatrix();
        glTranslatef(desloca_x + 40,0,0);
        glColor3f(color_x, color_y, color_z);
        glTranslatef(90.0f, 0.0f, 50.0f);
        glRotated(90, 0, 0, 1);
        //ajuste conforme tamanho da caixa
        glScalef(5, 40.0f, 70);
        glutSolidCube(3);
        glPopMatrix();
    }
}

/**
 * desenha um chão
 */
void desenha_chao()
{
    float color_x = 0.59;
    float color_y = 0.59;
    float color_z = 0.59;

    glPushMatrix();
    glTranslatef(150, 0, -50);
    glColor3f(color_x, color_y, color_z);
    glScalef(550.0f, 350.0f, 0);
    glutSolidCube(1);
    glPopMatrix();
}

/**
 * executa ações do menu
 */
void menu(int acao)
{
    ponto_3d rotacao_aux, escala_aux;
    int tipo, lugar = 0;

    rotacao_aux.x = 0.0;
    rotacao_aux.y = 0.0;
    rotacao_aux.z = 0.0;

    escala_aux.x = 0.0;
    escala_aux.y = 0.0;
    escala_aux.z = 0.0;

    switch (acao)
    {
    //sair do programa
    case -1:
    {
        cleanup();
        exit(0);
        break;
    }
    //cria mesa
    case 0:
    {
        tipo = 1;
        lugar = 3;
        cria_objeto(posicao_default_chao, rotacao_aux, escala_aux, tipo, lugar);

        break;
    }
    //cria cadeira
    case 1:
    {
        tipo = 0;
        lugar = 3;
        cria_objeto(posicao_default_chao, rotacao_aux, escala_aux, tipo, lugar);

        break;
    }
    //cria quadro na parede da esquerda
    case 2:
    {
        tipo = 2;
        lugar = 0;
        cria_objeto(posicao_default_parede_da_esquerda, rotacao_aux, escala_aux, tipo, lugar);

        break;
    }
    //cria quadro na parede da direita
    case 3:
    {
        tipo = 2;
        lugar = 1;
        cria_objeto(posicao_default_parede_da_direita, rotacao_aux, escala_aux, tipo, lugar);

        break;
    }
    //cria quadro na parede do fundo
    case 4:
    {
        tipo = 2;
        lugar = 2;
        cria_objeto(posicao_default_parede_do_fundo, rotacao_aux, escala_aux, tipo, lugar);

        break;
    }
    //cria quadro na parede do fundo
    case 5:
    {
        lista_objetos[objeto_selecionado].posicao.x = lista_objetos[objeto_selecionado].x_padrao;
        lista_objetos[objeto_selecionado].posicao.y = lista_objetos[objeto_selecionado].y_padrao;
        lista_objetos[objeto_selecionado].posicao.z = lista_objetos[objeto_selecionado].z_padrao;

        break;
    }
    //mostrar/ocultar paredes
    case 6:
    {
        if(mostrar_paredes_e_chao == TRUE)
        {
            mostrar_paredes_e_chao = FALSE;
        }
        else if(mostrar_paredes_e_chao == FALSE)
        {
            mostrar_paredes_e_chao = TRUE;
        }

        break;
    }
    }

    glutPostRedisplay();
}

/**
 * cria menu
 */
void cria_menu(void)
{
    int menu_principal, menu_objetos;
    int menu_posicoes_quadro;

    menu_posicoes_quadro = glutCreateMenu(menu);
    glutAddMenuEntry("Parede da Esquerda", 2);
    glutAddMenuEntry("Parede da Direita", 3);
    glutAddMenuEntry("Parede do Fundo", 4);

    menu_objetos = glutCreateMenu(menu);
    glutAddSubMenu("Quadro", menu_posicoes_quadro);
    glutAddMenuEntry("Cadeira", 1);
    glutAddMenuEntry("Mesa", 0);

    menu_principal = glutCreateMenu(menu);
    glutAddSubMenu("Criar um novo objeto", menu_objetos);
    glutAddMenuEntry("Reposicionar objeto", 5);
    glutAddMenuEntry("Mostrar/ocultar paredes", 6);
    glutAddMenuEntry("Sair", -1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/**
 * chama a função desenha_objeto a partir da posição relativa entre um objeto e um marcador
 */
static void draw(double trans1[3][4], double trans2[3][4], int posicao_do_objeto_na_lista)
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

    double gl_para[16];
    GLfloat mat_ambient[] = {0.0, 0.0, 1.0, 1.0};
    GLfloat mat_ambient1[] = {1.0, 0.0, 0.0, 1.0};
    GLfloat mat_flash[] = {0.0, 0.0, 1.0, 1.0};
    GLfloat mat_flash1[] = {1.0, 0.0, 0.0, 1.0};
    GLfloat mat_flash_shiny[] = {50.0};
    GLfloat mat_flash_shiny1[] = {50.0};
    GLfloat light_position[] = {100.0, -200.0, 200.0, 0.0};
    GLfloat ambi[] = {0.1, 0.1, 0.1, 0.1};
    GLfloat lightZeroColor[] = {0.9, 0.9, 0.9, 0.1};

    argDrawMode3D();
    argDraw3dCamera(0, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //load the camera transformation matrix
    glMatrixMode(GL_MODELVIEW);
    argConvGlpara(trans1, gl_para);
    glLoadMatrixd(gl_para);
    argConvGlpara(trans2, gl_para);
    glMultMatrixd(gl_para);

    //a cor do objeto selecionado pelo usuário é azul
    if (posicao_do_objeto_na_lista == objeto_selecionado)
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
    glMatrixMode(GL_MODELVIEW);

    //aplica a translação relativa entre marcador e objeto
    glTranslatef(x_relativo, y_relativo, z_relativo);

    //desenha objeto
    desenha_objeto(posicao_do_objeto_na_lista);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

/**
 * chama as funções de desenho para os objetos estáticos (chão, paredes e externos)
 */
static void draw_objetos_estaticos(double trans1[3][4], double trans2[3][4])
{
    //faz leitura das coordenadas do marcador
    double x_marcador = trans2[0][3];
    double y_marcador = trans2[1][3];
    double z_marcador = trans2[2][3];

    double gl_para[16];
    GLfloat mat_ambient[] = {0.0, 0.0, 1.0, 1.0};
    GLfloat mat_ambient1[] = {1.0, 0.0, 0.0, 1.0};
    GLfloat mat_ambient_x[] = {1.0, 1.0, 1.0, 0.0};
    GLfloat mat_flash[] = {0.0, 0.0, 1.0, 1.0};
    GLfloat mat_flash1[] = {1.0, 0.0, 0.0, 1.0};
    GLfloat mat_flash_x[] = {3.0, 3.0, 3.0, 3.0};
    GLfloat mat_flash_shiny[] = {50.0};
    GLfloat mat_flash_shiny1[] = {50.0};
    GLfloat mat_flash_shiny_x[] = {100.0};
    GLfloat light_position[] = {100.0, -200.0, 200.0, 0.0};
    GLfloat ambi[] = {0.1, 0.1, 0.1, 0.1};
    GLfloat lightZeroColor[] = {0.9, 0.9, 0.9, 0.1};

    argDrawMode3D();
    argDraw3dCamera(0, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //load the camera transformation matrix
    glMatrixMode(GL_MODELVIEW);
    argConvGlpara(trans1, gl_para);
    glLoadMatrixd(gl_para);
    argConvGlpara(trans2, gl_para);
    glMultMatrixd(gl_para);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash_x);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny_x);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_x);

    glMatrixMode(GL_MODELVIEW);

    //desenha chão
    glPushMatrix();
    glTranslatef(X_PAREDE_DA_ESQUERDA - x_marcador, Y_PAREDE_DO_FUNDO - y_marcador, Z_CHAO - z_marcador);
    desenha_chao();
    glPopMatrix();

    //abre região de oclusão
    if(mostrar_paredes_e_chao == FALSE)
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }

    //desenha parede da esquerda
    glPushMatrix();
    glTranslatef(X_PAREDE_DA_ESQUERDA - x_marcador, Y_PAREDE_DO_FUNDO - y_marcador, Z_CHAO - z_marcador);
    desenha_parede(0);
    glPopMatrix();

    //desenha parede da direita
    glPushMatrix();
    glTranslatef(X_PAREDE_DA_DIREITA - x_marcador, Y_PAREDE_DO_FUNDO - y_marcador, Z_CHAO - z_marcador);
    desenha_parede(1);
    glPopMatrix();

    //desenha parede do fundo
    glPushMatrix();
    glTranslatef(X_PAREDE_DA_ESQUERDA - x_marcador, Y_PAREDE_DO_FUNDO - y_marcador, Z_CHAO - z_marcador);
    glTranslatef(0,100,0);
    desenha_parede(2);
    glPopMatrix();

    //fecha região de oclusão
    if(mostrar_paredes_e_chao == FALSE)
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

    //desenha cubo atrás da sala
    glPushMatrix();
    glTranslatef((0) - x_marcador, (1000) - y_marcador, Z_CHAO - z_marcador);
    glutSolidCube(400);
    glPopMatrix();

    //desenha cubo no lado esquerdo da sala
    glPushMatrix();
    glTranslatef((-400) - x_marcador, Y_PAREDE_DO_FUNDO - y_marcador, Z_CHAO - z_marcador);
    glutSolidCube(250);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

/**
 * captura um frame, identifica marcadores visíveis e chama as funções draw
 */
static void mainLoop(void)
{
    glLoadIdentity();

    ARUint8 *dataPtr;
    ARMarkerInfo *marker_info;
    int marker_num;
    double err;
    int i;

    //captura um frame
    if ((dataPtr = (ARUint8 *)arVideoGetImage()) == NULL)
    {
        arUtilSleep(2);
        return;
    }
    if (count == 0)
        arUtilTimerReset();
    count++;

    //detecta os marcadores capturados n frame
    if (arDetectMarkerLite(dataPtr, thresh, &marker_info, &marker_num) < 0)
    {
        cleanup();
        exit(0);
    }

    argDrawMode2D();
    if (!arDebug)
    {
        argDispImage(dataPtr, 0, 0);
    }
    else
    {
        argDispImage(dataPtr, 1, 1);
        if (arImageProcMode == AR_IMAGE_PROC_IN_HALF)
        {
            argDispHalfImage(arImage, 0, 0);
        }
        else
        {
            argDispImage(arImage, 0, 0);
        }

        glColor3f(1.0, 0.0, 0.0);
        glLineWidth(1.0);
        for (i = 0; i < marker_num; i++)
        {
            argDrawSquare(marker_info[i].vertex, 0, 0);
        }
        glLineWidth(1.0);
    }

    arVideoCapNext();

    if ((err = arMultiGetTransMat(marker_info, marker_num, config)) < 0)
    {
        argSwapBuffers();
        return;
    }
    //printf("err = %f\n", err);
    if (err > 100.0)
    {
        argSwapBuffers();
        return;
    }

    argDrawMode3D();
    argDraw3dCamera(0, 0);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);

    //passa por todos os marcadores
    for (i = 0; i < config->marker_num; i++)
    {
        //se o marcador i está visível no frame
        //if ((config->marker[i].visible >= 0) && (marker_info->id == i))
        if (config->marker[i].visible >= 0)
        {
            int posicao_objeto;

            //desenha objetos estaticos
            draw_objetos_estaticos(config->trans, config->marker[i].trans);

            //desenha todos os objetos de maneira relativa ao marcador i
            for (posicao_objeto = 0; posicao_objeto < contador_objetos; posicao_objeto++)
            {
                glPushMatrix();

                draw(config->trans, config->marker[i].trans, posicao_objeto);

                glPopMatrix();
            }
        }
    }

    argSwapBuffers();
}

/**
 * inicializa variáveis e estruturas do programa
 */
static void init(void)
{
    ARParam wparam;

    //abre o caminha do vídeo
    if (arVideoOpen(vconf) < 0)
        exit(0);
    //encontra o tamanho da janela
    if (arVideoInqSize(&xsize, &ysize) < 0)
        exit(0);
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    //inicializa os parâmetros da câmera
    if (arParamLoad(cparam_name, 1, &wparam) < 0)
    {
        printf("Camera parameter load error !!\n");
        exit(0);
    }
    arParamChangeSize(&wparam, xsize, ysize, &cparam);
    arInitCparam(&cparam);
    printf("*** Camera Parameter ***\n");
    arParamDisp(&cparam);

    if ((config = arMultiReadConfigFile(config_name)) == NULL)
    {
        printf("config data load error !!\n");
        exit(0);
    }

    //abre a janela
    argInit(&cparam, 1.0, 0, 0, 0, 0);
    arFittingMode = AR_FITTING_TO_IDEAL;
    arImageProcMode = AR_IMAGE_PROC_IN_HALF;
    argDrawMode = AR_DRAW_BY_TEXTURE_MAPPING;
    argTexmapMode = AR_DRAW_TEXTURE_HALF_IMAGE;

    //define posições padrões
    posicao_default_parede_da_esquerda.x = X_PAREDE_DA_ESQUERDA;
    posicao_default_parede_da_esquerda.y = -100;
    posicao_default_parede_da_esquerda.z = 40;

    posicao_default_parede_da_direita.x = X_PAREDE_DA_DIREITA;
    posicao_default_parede_da_esquerda.y = -100;
    posicao_default_parede_da_esquerda.z = 40;

    posicao_default_parede_do_fundo.x = 137;
    posicao_default_parede_do_fundo.y = Y_PAREDE_DO_FUNDO;
    posicao_default_parede_do_fundo.z = 40;

    posicao_default_chao.x = 137;
    posicao_default_chao.y = -110;
    posicao_default_chao.z = Z_CHAO;
}

/**
 * efetua procedimentos necessários para o encerramento do programa
 */
static void cleanup(void)
{
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}

/**
 * tratamento das teclas pressionadas
 */
static void keyEvent(unsigned char tecla, int x, int y)
{
    //ESC => encerra programa
    if (tecla == 0x1b)
    {
        cleanup();
        exit(0);
    }
    //r => deleta objeto selecionado
    else if (tecla == 'r' || tecla == 'R')
    {
        int id_do_objeto_a_ser_removido = lista_objetos[objeto_selecionado].id;
        remove_objeto(id_do_objeto_a_ser_removido);
    }
    //a => movimenta objeto
    else if (tecla == 'a' || tecla == 'A')
    {
        //se o objeto está na parede da esquerda, z--
        if (lista_objetos[objeto_selecionado].lugar[0] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.z;
            novo_valor--;
            lista_objetos[objeto_selecionado].posicao.z = novo_valor;
        }
        //se o objeto está na parede da direita, z--
        else if (lista_objetos[objeto_selecionado].lugar[1] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.z;
            novo_valor--;
            lista_objetos[objeto_selecionado].posicao.z = novo_valor;
        }
        //se o objeto está na parede do fundo, x--
        else if (lista_objetos[objeto_selecionado].lugar[2] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.x;
            novo_valor--;
            lista_objetos[objeto_selecionado].posicao.x = novo_valor;
        }
        //se o objeto está no chão, x--
        else if (lista_objetos[objeto_selecionado].lugar[3] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.x;
            novo_valor--;
            lista_objetos[objeto_selecionado].posicao.x = novo_valor;
        }
    }
    //d => movimenta objeto
    else if (tecla == 'd' || tecla == 'D')
    {
        //se o objeto está na parede da esquerda, z++
        if (lista_objetos[objeto_selecionado].lugar[0] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.z;
            novo_valor++;
            lista_objetos[objeto_selecionado].posicao.z = novo_valor;
        }
        //se o objeto está na parede da direita, z++
        else if (lista_objetos[objeto_selecionado].lugar[1] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.z;
            novo_valor++;
            lista_objetos[objeto_selecionado].posicao.z = novo_valor;
        }
        //se o objeto está na parede do fundo, x++
        else if (lista_objetos[objeto_selecionado].lugar[2] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.x;
            novo_valor++;
            lista_objetos[objeto_selecionado].posicao.x = novo_valor;
        }
        //se o objeto está no chão, x++
        else if (lista_objetos[objeto_selecionado].lugar[3] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.x;
            novo_valor++;
            lista_objetos[objeto_selecionado].posicao.x = novo_valor;
        }
    }
    //w => movimenta objeto
    else if (tecla == 'w' || tecla == 'W')
    {
        //se o objeto está na parede da esquerda, y++
        if (lista_objetos[objeto_selecionado].lugar[0] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.y;
            novo_valor++;
            lista_objetos[objeto_selecionado].posicao.y = novo_valor;
        }
        //se o objeto está na parede da direita, y++
        else if (lista_objetos[objeto_selecionado].lugar[1] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.y;
            novo_valor++;
            lista_objetos[objeto_selecionado].posicao.y = novo_valor;
        }
        //se o objeto está na parede do fundo, z++
        else if (lista_objetos[objeto_selecionado].lugar[2] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.z;
            novo_valor++;
            lista_objetos[objeto_selecionado].posicao.z = novo_valor;
        }
        //se o objeto está no chão, y++
        else if (lista_objetos[objeto_selecionado].lugar[3] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.y;
            novo_valor++;
            lista_objetos[objeto_selecionado].posicao.y = novo_valor;
        }
    }
    //s => movimenta objeto
    else if (tecla == 's' || tecla == 'S')
    {
        //se o objeto está na parede da esquerda, y--
        if (lista_objetos[objeto_selecionado].lugar[0] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.y;
            novo_valor--;
            lista_objetos[objeto_selecionado].posicao.y = novo_valor;
        }
        //se o objeto está na parede da direita, y--
        else if (lista_objetos[objeto_selecionado].lugar[1] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.y;
            novo_valor--;
            lista_objetos[objeto_selecionado].posicao.y = novo_valor;
        }
        //se o objeto está na parede do fundo, z--
        else if (lista_objetos[objeto_selecionado].lugar[2] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.z;
            novo_valor--;
            lista_objetos[objeto_selecionado].posicao.z = novo_valor;
        }
        //se o objeto está no chão, y--
        else if (lista_objetos[objeto_selecionado].lugar[3] == TRUE)
        {
            double novo_valor = lista_objetos[objeto_selecionado].posicao.y;
            novo_valor--;
            lista_objetos[objeto_selecionado].posicao.y = novo_valor;
        }
    }
    //q => seleciona objeto anterior
    else if (tecla == 'q' || tecla == 'Q')
    {
        if (objeto_selecionado > 0)
        {
            objeto_selecionado--;
        }
    }
    //e => seleciona próximo objeto
    else if (tecla == 'e')
    {
        if (objeto_selecionado < contador_objetos - 1)
        {
            objeto_selecionado++;
        }
    }
}

/**
 * função main
 */
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    init();
    cria_menu();

    arVideoCapStart();
    argMainLoop(NULL, keyEvent, mainLoop);
    return (0);
}
