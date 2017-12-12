#include <math.h>
#include <time.h>
#include <cstdlib>
#include <stdio.h>
#include <assert.h>
#include <glm/vec3.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>

const glm::vec3 EYE = glm::vec3(0.0, -10.0, 20.0);
const float FIELD_OF_VIEW = M_PI * 0.6;
const int IMAGE_SIZE = 500;
const int Z_LIM = 100;

float flmod(float fl, float m) {
    return fl - m * floor(fl / m);
}

float userFunction(float x, float y, float z) {
    return z - fmax(0.0f, 16.0f - 0.3f * (x*x + y*y)); // Paraboloide
    return z - fmax(0.0f, 25.0f - 0.1f * (x*x + y*y)); // Otro paraboloide
    return z + fmax(fabs(flmod(x, 10.0f)-5.0f), fabs(flmod(y, 10.0f)-5.0f)); // Estoperoles
    return z > 0.0f ? x*x + y*y + z*z - 70.0f : z; // Esfera
}

float evaluateUserFunction(glm::vec3 ray, float multiplier) {
    glm::vec3 pointInRay = EYE + ray * multiplier;
    return userFunction(pointInRay.x, pointInRay.y, pointInRay.z) ;
}

float findFunctionRoot(glm::vec3 ray, float pointOnPositiveSide, float pointOnNegativeSide) {
    assert(pointOnNegativeSide >= pointOnPositiveSide);  
    float mid ;
    for (int step = 0; step < 55; ++step) { //52bits
        mid = (pointOnNegativeSide + pointOnPositiveSide) / 2 ;
        if (evaluateUserFunction(ray, mid) < 0) 
            pointOnNegativeSide = mid ; 
        else
            pointOnPositiveSide = mid ;
    }
    return mid;
}

int parity(float a) {
    int intPart = (int)floor(a) ;
    return !(intPart % 2 == 0);
}

bool castRay(glm::vec3 ray) {
    // Encontrar puntos en la trayectoria del rayo tales que userFunction(ray * pointOnPositiveSide) > 0 y userFunction(ray * pointOnNegativeSide) < 0.
    assert(evaluateUserFunction(EYE, 0.0f) > 0.0f and "EYE should be on positive side of function.");  
    int iterations = 500 ;
    float pointOnNegativeSide = Z_LIM;
    bool findNegative = evaluateUserFunction(ray, pointOnNegativeSide) < 0.0f;

    while (iterations--) {
        float randomMultiplier = ((float) rand() / RAND_MAX) * pointOnNegativeSide;
        if (evaluateUserFunction(ray, randomMultiplier) < 0) {
            pointOnNegativeSide = randomMultiplier;
            findNegative = true;
        }
    }
    if (not findNegative){
        return false;
    }
    float functionRoot = findFunctionRoot(ray, 0.0f /* pointOnPositiveSide */, pointOnNegativeSide);
    glm::vec3 root = EYE + ray * ((float)functionRoot) ; // Busqueda binaria.
    return parity(root.x) == parity(root.y); // Regresar falso si cae en cuadrito blanco, verdadero si cae en cuadrito negro.
}

int main(int argc, char** argv) {
    srand(time(NULL)) ;
    // Compute coordinates of the corners of the virtual screen.
    glm::vec3 dir = glm::normalize(-EYE);
    glm::vec3 left = glm::normalize(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(dir, left));
    glm::vec3 topLeft = EYE + dir - left - up;
    glm::vec3 topRight = EYE + dir + left - up;
    glm::vec3 bottomLeft = EYE + dir - left + up;
    glm::vec3 bottomRight = EYE + dir + left + up;

    // Cast rays onto function and generate "chess board" image.
    bool chessBoard[IMAGE_SIZE][IMAGE_SIZE];
    for (int pixelY = 0; pixelY < IMAGE_SIZE; pixelY ++) {
        for (int pixelX = 0; pixelX < IMAGE_SIZE; pixelX ++) {
            glm::vec3 pixelReference =
                    topLeft + (topRight - topLeft) * ((float) pixelX / IMAGE_SIZE )
                            + (bottomLeft - topLeft) * ((float) pixelY / IMAGE_SIZE );
            glm::vec3 pixelRay = glm::normalize(pixelReference - EYE);
            chessBoard[pixelY][pixelX] = castRay(pixelRay);
        }
    }
  
    // Detect borders on "chess board".
    bool bordersImage[IMAGE_SIZE][IMAGE_SIZE];
    for (int pixelY = 1; pixelY < IMAGE_SIZE; pixelY ++) {
        for (int pixelX = 1; pixelX < IMAGE_SIZE; pixelX ++) {
            // TODO: Improve this.
            bordersImage[pixelY][pixelX] = 
                    chessBoard[pixelY][pixelX] != chessBoard[pixelY-1][pixelX]
                            || chessBoard[pixelY][pixelX] != chessBoard[pixelY][pixelX-1];
        }
    }

    // Save bordersImage to file.
    printf("P2\n");
    printf("%d %d\n", IMAGE_SIZE, IMAGE_SIZE);
    printf("1\n");
    for (int i = 0; i < IMAGE_SIZE; i ++) {
      for (int j = 0; j < IMAGE_SIZE; j ++) {
//        printf("%d ", chessBoard[i][j] ? 0 : 1);
        printf("%d ", bordersImage[i][j] ? 0 : 1);
      }
      printf("\n");
    }

    return 0;
}
