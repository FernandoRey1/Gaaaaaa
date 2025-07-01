#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <GLUT/glut.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif

#include <vector>
#include <ctime>
#include <cmath>
#include <string>     // Para std::string
#include <algorithm>  // Para std::remove_if

const int ANCHO_MAPA = 21;
const int ALTO_MAPA = 21;
const int TAMANO_CELDA = 30;
int anchoVentana = ANCHO_MAPA * TAMANO_CELDA;
int altoVentana = ALTO_MAPA * TAMANO_CELDA;

// Estado del juego
bool juegoTerminado = false;
bool juegoGanado = false;
int puntuacion = 0;
int vidas = 3;

// Estructuras
struct Posicion {
	int x, y;
};

Posicion pacman;
std::vector<Posicion> fantasmas;
std::vector<Posicion> puntos;
std::vector<Posicion> puntosGrandes;

enum Direccion { DETENIDO, IZQUIERDA, DERECHA, ARRIBA, ABAJO };
Direccion direccion = DETENIDO;
Direccion siguienteDireccion = DETENIDO;

// Mapa del juego (1 = pared, 0 = camino)
int mapa[ALTO_MAPA][ANCHO_MAPA] = {
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{1,0,1,1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,0,1},
{1,0,1,1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,1,1,0,1,0,1,1,1,1,1,1,0,1,0,1,1,0,0,1},
	{1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
{1,1,1,1,0,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1},
	{1,1,1,1,0,1,0,0,0,0,0,0,0,0,1,0,1,1,1,1,1},
{1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,1},
	{1,0,1,1,0,1,0,1,1,1,1,1,1,0,1,0,1,1,0,0,1},
{1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
	{1,0,1,1,0,1,0,1,1,1,1,1,1,0,1,0,1,1,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,1,1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,0,1},
{1,0,1,1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,0,1},
	{1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

void reshape_cb(int w, int h) {
	if (w == 0 || h == 0) return;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void dibujarCuadrado(int x, int y, float r, float g, float b) {
	glColor3f(r, g, b);
	glBegin(GL_QUADS);
	glVertex2i(x * TAMANO_CELDA, y * TAMANO_CELDA);
	glVertex2i((x + 1) * TAMANO_CELDA, y * TAMANO_CELDA);
	glVertex2i((x + 1) * TAMANO_CELDA, (y + 1) * TAMANO_CELDA);
	glVertex2i(x * TAMANO_CELDA, (y + 1) * TAMANO_CELDA);
	glEnd();
}

void dibujarCirculo(float x, float y, float radio, float r, float g, float b) {
	glColor3f(r, g, b);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y); // Centro del círculo
	for (int i = 0; i <= 360; i++) {
		float angulo = i * 3.14159f / 180.0f;
		glVertex2f(x + cos(angulo) * radio, y + sin(angulo) * radio);
	}
	glEnd();
}

void inicializarJuego() {
	// Inicializar Pacman
	pacman.x = 10;
	pacman.y = 15;
	
	// Inicializar fantasmas
	fantasmas.clear();
	fantasmas.push_back({9, 9});
	fantasmas.push_back({10, 9});
	fantasmas.push_back({11, 9});
	fantasmas.push_back({12, 9});
	
	// Inicializar puntos
	puntos.clear();
	puntosGrandes.clear();
	for (int y = 0; y < ALTO_MAPA; y++) {
		for (int x = 0; x < ANCHO_MAPA; x++) {
			if (mapa[y][x] == 0) {
				if ((x == 1 && y == 1) || (x == ANCHO_MAPA-2 && y == 1) || 
					(x == 1 && y == ALTO_MAPA-2) || (x == ANCHO_MAPA-2 && y == ALTO_MAPA-2)) {
					puntosGrandes.push_back({x, y});
				} else {
						puntos.push_back({x, y});
					}
			}
		}
	}
	
	puntuacion = 0;
	vidas = 3;
}

void dibujar() {
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Dibujar mapa
	glColor3f(0.0f, 0.0f, 0.3f); // Azul oscuro para el fondo
	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(anchoVentana, 0);
	glVertex2i(anchoVentana, altoVentana);
	glVertex2i(0, altoVentana);
	glEnd();
	
	// Dibujar paredes
	glColor3f(0.0f, 0.0f, 1.0f); // Azul para las paredes
	for (int y = 0; y < ALTO_MAPA; y++) {
		for (int x = 0; x < ANCHO_MAPA; x++) {
			if (mapa[y][x] == 1) {
				dibujarCuadrado(x, y, 0.0f, 0.0f, 1.0f);
			}
		}
	}
	
	// Dibujar puntos
	for (auto p : puntos) {
		dibujarCirculo((p.x + 0.5) * TAMANO_CELDA, (p.y + 0.5) * TAMANO_CELDA, 
			TAMANO_CELDA/8, 1.0f, 1.0f, 0.0f);
	}
	
	// Dibujar puntos grandes
	for (auto p : puntosGrandes) {
		dibujarCirculo((p.x + 0.5) * TAMANO_CELDA, (p.y + 0.5) * TAMANO_CELDA, 
			TAMANO_CELDA/4, 1.0f, 1.0f, 0.0f);
	}
	
	// Dibujar Pacman
	dibujarCirculo((pacman.x + 0.5) * TAMANO_CELDA, (pacman.y + 0.5) * TAMANO_CELDA, 
		 TAMANO_CELDA/2, 1.0f, 1.0f, 0.0f); // Amarillo para Pacman
	
	// Dibujar fantasmas
	for (auto fantasma : fantasmas) {
		dibujarCirculo((fantasma.x + 0.5) * TAMANO_CELDA, 
			(fantasma.y + 0.5) * TAMANO_CELDA, 
			TAMANO_CELDA/2, 1.0f, 0.0f, 0.0f); // Rojo para los fantasmas
	}
	
	// Dibujar información del juego
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2i(10, altoVentana - 20);
	std::string texto = "Puntuación: " + std::to_string(puntuacion) + "   Vidas: " + std::to_string(vidas);
	for (char c : texto) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
	}
	
	if (juegoTerminado) {
		glColor3f(1.0f, 0.0f, 0.0f);
		glRasterPos2i(anchoVentana/2 - 50, altoVentana/2);
		std::string mensaje = juegoGanado ? "¡GANASTE!" : "¡JUEGO TERMINADO!";
		for (char c : mensaje) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
		}
	}
	
	glutSwapBuffers();
}

void tecladoEspecial(int tecla, int x, int y) {
	switch (tecla) {
	case GLUT_KEY_UP: siguienteDireccion = ARRIBA; break;
	case GLUT_KEY_DOWN: siguienteDireccion = ABAJO; break;
	case GLUT_KEY_LEFT: siguienteDireccion = IZQUIERDA; break;
	case GLUT_KEY_RIGHT: siguienteDireccion = DERECHA; break;
	}
}

void moverFantasmas() {
	if (juegoTerminado) return;
	
	for (auto& fantasma : fantasmas) {
		// Movimiento aleatorio simple de los fantasmas
		int dir = rand() % 4;
		int nx = fantasma.x, ny = fantasma.y;
		
		switch (dir) {
		case 0: nx--; break; // Izquierda
		case 1: nx++; break; // Derecha
		case 2: ny--; break; // Abajo
		case 3: ny++; break; // Arriba
		}
		
		// Verificar colisión con paredes
		if (nx >= 0 && nx < ANCHO_MAPA && ny >= 0 && ny < ALTO_MAPA && mapa[ny][nx] == 0) {
			fantasma.x = nx;
			fantasma.y = ny;
		}
	}
}

void verificarColisiones() {
	// Verificar colisión con puntos
	auto it = std::remove_if(puntos.begin(), puntos.end(), [&](Posicion p) {
		if (p.x == pacman.x && p.y == pacman.y) {
			puntuacion += 10;
			return true;
		}
		return false;
	});
	puntos.erase(it, puntos.end());
	
	// Verificar colisión con puntos grandes
	auto it2 = std::remove_if(puntosGrandes.begin(), puntosGrandes.end(), [&](Posicion p) {
		if (p.x == pacman.x && p.y == pacman.y) {
			puntuacion += 50;
			return true;
		}
		return false;
	});
	puntosGrandes.erase(it2, puntosGrandes.end());
	
	// Verificar colisión con fantasmas
	for (const auto& fantasma : fantasmas) {
		if (fantasma.x == pacman.x && fantasma.y == pacman.y) {
			vidas--;
			if (vidas <= 0) {
				juegoTerminado = true;
			} else {
				// Reubicar Pacman
				pacman.x = 10;
				pacman.y = 15;
				direccion = DETENIDO;
				siguienteDireccion = DETENIDO;
			}
			break;
		}
	}
	
	// Verificar si se ganó el juego
	if (puntos.empty() && puntosGrandes.empty()) {
		juegoTerminado = true;
		juegoGanado = true;
	}
}

void actualizar(int valor) {
	if (!juegoTerminado) {
		// Mover Pacman
		if (siguienteDireccion != DETENIDO) {
			int nx = pacman.x, ny = pacman.y;
			
			switch (siguienteDireccion) {
			case IZQUIERDA: nx--; break;
			case DERECHA: nx++; break;
			case ARRIBA: ny++; break;
			case ABAJO: ny--; break;
			case DETENIDO: break;
			}
			
			// Verificar si el movimiento es válido
			if (nx >= 0 && nx < ANCHO_MAPA && ny >= 0 && ny < ALTO_MAPA && mapa[ny][nx] == 0) {
				direccion = siguienteDireccion;
				pacman.x = nx;
				pacman.y = ny;
			} else {
				// Intentar continuar en la dirección actual
				nx = pacman.x; ny = pacman.y;
				switch (direccion) {
				case IZQUIERDA: nx--; break;
				case DERECHA: nx++; break;
				case ARRIBA: ny++; break;
				case ABAJO: ny--; break;
				case DETENIDO: break;
				}
				
				if (nx >= 0 && nx < ANCHO_MAPA && ny >= 0 && ny < ALTO_MAPA && mapa[ny][nx] == 0) {
					pacman.x = nx;
					pacman.y = ny;
				}
			}
		}
		
		moverFantasmas();
		verificarColisiones();
		
		glutPostRedisplay();
		glutTimerFunc(100, actualizar, 0);
	}
}

void display_cb() {
	dibujar();
}

void initialize() {
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(anchoVentana, altoVentana);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Pacman");
	glutDisplayFunc(display_cb);
	glutReshapeFunc(reshape_cb);
	glutSpecialFunc(tecladoEspecial);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	inicializarJuego();
	glutTimerFunc(100, actualizar, 0);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	initialize();
	glutMainLoop();
	return 0;
}

