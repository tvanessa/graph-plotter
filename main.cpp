#include <math.h>
#include <time.h>
#include <cstdlib>
#include <stdio.h>
#include <glm/vec3.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>

const glm::vec3 EYE = glm::vec3(0.0, -10.0, 20.0);
const float FIELD_OF_VIEW = M_PI * 0.6;
//const float FIELD_OF_VIEW = 108.0f ;
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

float findFunctionRoot(glm::vec3 ray, float pointOnPositiveSide, float pointOnNegativeSide) {
    float mid ;
    if (pointOnNegativeSide >= pointOnPositiveSide) {
        for (int step = 0; step < 55; ++step) { //52bits
            mid = (pointOnNegativeSide + pointOnPositiveSide) / 2 ;
            glm::vec3 mid_point = EYE + (ray * ((float)mid)) ;
            float mid_evaluate = userFunction(mid_point.x, mid_point.y, mid_point.z) ;     

            if (mid_evaluate < 0) 
                pointOnNegativeSide = mid ; 
            else pointOnPositiveSide = mid ;
            /*if (step == 54)
                fprintf(stderr, "f(root) = %f\n", mid_evaluate);*/
        }
    }
    return mid ; // 
}

float generateRandom(float left, float right) {
    float _random = (float)rand() / RAND_MAX ;
    return left + _random * (right - left) ;
}

int parity(float a) {
    int n = (int)floor(a) ;
    if (n % 2 == 0) //even
        return 0 ;
    else
        return 1 ;
}

float evaluateUserFunction(glm::vec3 pointInRay) {
    return userFunction(pointInRay.x, pointInRay.y, pointInRay.z) ;
}

bool castRay(glm::vec3 ray) {
    // Encontrar puntos en la trayectoria del rayo tales que userFunction(ray * pointOnPositiveSide) > 0 y userFunction(ray * pointOnNegativeSide) < 0.
    int iterations = 500 ;
    float pointOnPositiveSide = 0;
    float pointOnNegativeSide = Z_LIM;
    bool findNegative = evaluateUserFunction(EYE + ray * pointOnPositiveSide) > 0.0f;
    bool findPositive = evaluateUserFunction(EYE + ray * pointOnNegativeSide) < 0.0f;

    while (iterations--) {
        float _random = generateRandom(0, pointOnNegativeSide) ;
        glm::vec3 point = EYE + ray * ((float)_random) ;
        float evaluate = evaluateUserFunction(point) ;
        if (evaluate < 0) {
            float tmpNegative = _random ;
            if (tmpNegative < pointOnNegativeSide){
                pointOnNegativeSide = tmpNegative ;
                findNegative = true ;
            }
        }
    }
    
    /*glm::vec3 pointP = EYE + ray * ((float)pointOnPositiveSide) ;
    float eP = userFunction(pointP.x, pointP.y, pointP.z) ;
    fprintf(stderr, "f(positive) = %f\n", eP);

    glm::vec3 pointN = EYE + ray * ((float)pointOnNegativeSide) ;
    float eN = userFunction(pointN.x, pointN.y, pointN.z) ;
    fprintf(stderr, "f(Negative) = %f\n", eN);*/

    if ((findPositive == true) && (findNegative == true)){
        float functionRoot = findFunctionRoot(ray, pointOnPositiveSide, pointOnNegativeSide);
        glm::vec3 root = EYE + ray * ((float)functionRoot) ; // Busqueda binaria.
        //fprintf(stderr, "(%.3f, %.3f, %.3f)  ", root.x, root.y, root.z);
        // Regresar falso si cae en cuadrito blanco, verdadero si cae en cuadrito negro.
        if (parity(root.x) == parity(root.y))
            return true ;
        else
            return false ;

    } else {
     if (findPositive == false) 
            fprintf(stderr, "Posive side not found\n");
        if (findNegative == false)
            fprintf(stderr, "Negative side not found\n");
    }
}

int main(int argc, char** argv) {
    srand(time(NULL)) ;
    /*fprintf(stderr, "2.1: %f\n", fabs(2.1));
    fprintf(stderr, "-100.3: %f\n", fabs(-100.3));
    fprintf(stderr, "-100.3: %d\n", (int)floor(fabs(-100.3)));
    fprintf(stderr, "-500.001: %f\n", fabs(-500.001));
    fprintf(stderr, "-500.001: %d\n", (int)floor(fabs(-500.001)));*/
    // Compute coordinates of the corners of the virtual screen.
    glm::vec3 dir = glm::normalize(-EYE);
    glm::vec3 left = glm::normalize(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(dir, left));
    glm::vec3 topLeft = EYE + dir - left - up;
    glm::vec3 topRight = EYE + dir + left - up;
    glm::vec3 bottomLeft = EYE + dir - left + up;
    glm::vec3 bottomRight = EYE + dir + left + up;
    fprintf(stderr, "norm: %f\n", glm::distance2(topLeft, topRight));
    fprintf(stderr, "norm: %f\n", glm::distance2(bottomLeft, bottomRight));
    fprintf(stderr, "norm: %f\n", glm::distance2(topLeft, bottomLeft));
    fprintf(stderr, "norm: %f\n", glm::distance2(topRight, bottomRight));
    fprintf(stderr, "norm: %f\n", glm::distance2(topLeft, bottomRight));
    fprintf(stderr, "norm: %f\n", glm::distance2(topRight, bottomLeft));

    // Cast rays onto function and generate "chess board" image.
    bool chessBoard[IMAGE_SIZE][IMAGE_SIZE];
    for (int pixelY = 0; pixelY < IMAGE_SIZE; pixelY ++) {
        for (int pixelX = 0; pixelX < IMAGE_SIZE; pixelX ++) {
            glm::vec3 pixelReference =
                    topLeft + (topRight - topLeft) * ((float) pixelX / IMAGE_SIZE )
                            + (bottomLeft - topLeft) * ((float) pixelY / IMAGE_SIZE );
            //fprintf(stderr, "(%.3f, %.3f, %.3f)  ", pixelReference.x, pixelReference.y, pixelReference.z);
            glm::vec3 pixelRay = glm::normalize(EYE - pixelReference);

            chessBoard[pixelY][pixelX] = castRay(-pixelRay);
        }
        //fprintf(stderr, "\n");
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
