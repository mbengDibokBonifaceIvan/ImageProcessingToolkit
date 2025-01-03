#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    unsigned char *data;
    int width;
    int height;
} Image;

// Fonction pour lire une image PGM
Image readPGM(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    Image img;
    char format[3];
    fscanf(file, "%s", format); // Lire le format
    fscanf(file, "%d %d", &img.width, &img.height); // Lire la largeur et la hauteur
    int maxVal;
    fscanf(file, "%d", &maxVal); // Lire la valeur maximale

    img.data = (unsigned char *)malloc(img.width * img.height);
    fread(img.data, sizeof(unsigned char), img.width * img.height, file);
    fclose(file);

    return img;
}

// Fonction pour écrire une image PGM par Ivan
// void writePGM(const char *filename, Image img) {
//     FILE *file = fopen(filename, "w");
//     fprintf(file, "P5\n%d %d\n255\n", img.width, img.height);
//     fwrite(img.data, sizeof(unsigned char), img.width * img.height, file);
//     fclose(file);
// }

// Fonction pour écrire une image PGM par Amirah
int writePGM(const char *filename, Image img) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return 1; // Erreur lors de l'ouverture du fichier
    }

    fprintf(file, "P5\n%d %d\n255\n", img.width, img.height);
    if (fwrite(img.data, sizeof(unsigned char), img.width * img.height, file) != img.width * img.height) {
        fclose(file);
        return 2; // Erreur lors de l'écriture des données
    }

    fclose(file);
    return 0; // Écriture réussie
}

// Fonction pour additionner deux images
Image addImages(Image img1, Image img2) {
    Image result;
    result.width = img1.width;
    result.height = img1.height;
    result.data = (unsigned char *)malloc(result.width * result.height);

    for (int i = 0; i < result.width * result.height; i++) {
        int sum = img1.data[i] + img2.data[i];
        result.data[i] = (sum > 255) ? 255 : sum; // Clamping à 255
    }

    return result;
}


// Fonction pour modifier le contraste d'une image
Image modifyContrast(Image img, float contrastFactor) {
    Image result;
    result.width = img.width;
    result.height = img.height;
    result.data = (unsigned char *)malloc(result.width * result.height);

    for (int i = 0; i < result.width * result.height; i++) {
        int newPixel = (int)(img.data[i] * contrastFactor);
        result.data[i] = (newPixel > 255) ? 255 : (newPixel < 0) ? 0 : newPixel;
    }

    return result;
}

// Fonction pour créer un noyau gaussien
float* createGaussianKernel(int size, float sigma) {
    float* kernel = (float*)malloc(size * size * sizeof(float));
    float sum = 0.0;
    for (int x = -size/2; x <= size/2; x++) {
        for (int y = -size/2; y <= size/2; y++) {
            kernel[x + size/2 + (y + size/2) * size] = exp(-(x*x + y*y) / (2*sigma*sigma));
            sum += kernel[x + size/2 + (y + size/2) * size];
        }
    }
    // Normalisation
    for (int i = 0; i < size * size; i++) {
        kernel[i] /= sum;
    }
    return kernel;
}

Image convolution(Image img, float* kernel, int kernelSize) {
    int halfSize = kernelSize / 2;
    Image result = img; // Copie de l'image d'origine

    for (int y = halfSize; y < img.height - halfSize; y++) {
        for (int x = halfSize; x < img.width - halfSize; x++) {
            float sum = 0.0;
            for (int ky = -halfSize; ky <= halfSize; ky++) {
                for (int kx = -halfSize; kx <= halfSize; kx++) {
                    sum += img.data[(y + ky) * img.width + (x + kx)] * kernel[(ky + halfSize) * kernelSize + (kx + halfSize)];
                }
            }
            result.data[y * img.width + x] = (unsigned char)fmin(fmax(sum, 0.0), 255.0);
        }
    }
    return result;
}


// Fonction pour appliquer un filtre passe-bas
Image applyLowPassFilter(Image img, int filterType, int param) {
    float* kernel = NULL;
    if (filterType == 1) { // Moyenneur
        // Créer un noyau moyenneur de taille param
        kernel = (float*)malloc(param * param * sizeof(float));
        float value = 1.0 / (param * param);
        for (int i = 0; i < param * param; i++) {
            kernel[i] = value;
        }
    } else if (filterType == 2) { // Gaussien
        kernel = createGaussianKernel(param, param / 3.0); // Variance ajustée en fonction de la taille
    } else {
        fprintf(stderr, "Type de filtre invalide.\n");
        exit(1);
    }

    Image result = convolution(img, kernel, param);
    free(kernel);
    return result;
}


// Fonction pour appliquer un filtre passe-haut
Image applyHighPassFilter(Image img, int filterType, float threshold) {
    float* kernel = NULL;
    int kernelSize = 3; // Taille des noyaux (ajustable si nécessaire)

    switch (filterType) {
        case 1: // Prewitt
            kernel = (float*)malloc(kernelSize * kernelSize * sizeof(float));
            float prewittX[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
            float prewittY[3][3] = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};
            // Combinaison des deux noyaux pour obtenir le gradient
            for (int i = 0; i < kernelSize; i++) {
                for (int j = 0; j < kernelSize; j++) {
                    kernel[i * kernelSize + j] = prewittX[i][j] + prewittY[i][j];
                }
            }
            break;
        case 2: // Roberts
            kernel = (float*)malloc(kernelSize * kernelSize * sizeof(float));
            float robertsX[2][2] = {{0, 1}, {-1, 0}};
            float robertsY[2][2] = {{1, 0}, {0, -1}};
            // Combinaison des deux noyaux pour obtenir le gradient
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    kernel[i * 2 + j] = robertsX[i][j] + robertsY[i][j];
                }
            }
            kernelSize = 2; // Ajustement de la taille du noyau
            break;
        case 3: // Laplacien
            kernel = (float*)malloc(kernelSize * kernelSize * sizeof(float));
            float laplacian[3][3] = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};
            for (int i = 0; i < kernelSize; i++) {
                for (int j = 0; j < kernelSize; j++) {
                    kernel[i * kernelSize + j] = laplacian[i][j];
                }
            }
            break;
        default:
            fprintf(stderr, "Type de filtre invalide.\n");
            exit(1);
    }
 // Appliquer la convolution
    Image gradient = convolution(img, kernel, kernelSize);

    // Calculer le module du gradient ou la valeur absolue du Laplacien
    for (int i = 0; i < gradient.width * gradient.height; i++) {
        gradient.data[i] = (abs(gradient.data[i]) > threshold) ? 255 : 0;
    }

    free(kernel);
    return gradient;
}

int main() {
    Image img1 = readPGM("Assets/lena.512.pgm");
    Image img2 = readPGM("Assets/cristaux.512.pgm");

    if (img1.width != img2.width || img1.height != img2.height) {
        fprintf(stderr, "Les images doivent avoir la même taille!\n");
        exit(EXIT_FAILURE);
    }


//Appliquons le contraste
float contrastFactor;
    printf("Entrez le facteur de contraste (1 pour ne rien changer) : ");
    if (scanf("%f", &contrastFactor) != 1) {
        fprintf(stderr, "Erreur lors de la saisie du facteur de contraste.\n");
        return 1; // Indique une erreur
    }

    Image resultcontrast = modifyContrast(img1, contrastFactor);
    writePGM("Assets/result_contrast.pgm", resultcontrast);

        if (resultcontrast.data == NULL) {
        fprintf(stderr, "Erreur lors de la modification du contraste.\n");
        return 1; // Indique une erreur
    }
    if (writePGM("Assets/result_contrast.pgm", resultcontrast) != 0) {
        fprintf(stderr, "Erreur lors de l'écriture de l'image.\n");
        return 1; // Indique une erreur
    }

    printf("Modification du contraste effectuée avec succès !\n");


//Appliquons le filtre passe bas
 int filterType, filterSize;
    printf("Choisissez le type de filtre (1: Moyenneur, 2: Gaussien) : ");
    scanf("%d", &filterType);
    printf("Entrez la taille du filtre ou la variance (selon le type) : ");
    scanf("%d", &filterSize);

    Image result_lowpassfilter = applyLowPassFilter(img1, filterType, filterSize);

    if (writePGM("Assets/result_filtered.pgm", result_lowpassfilter) == 0) {
        printf("Le filtrage a été effectué avec succès.\n");
    } else {
        fprintf(stderr, "Erreur lors de l'écriture de l'image filtrée.\n");
    }


//Appliquons le filtre passe haut

    int highfilterType;
    float highthreshold;
    printf("Choisissez le type de filtre (1: Prewitt, 2: Roberts, 3: Laplacien) : ");
    scanf("%d", &highfilterType);
    printf("Entrez la valeur du seuil : ");
    scanf("%f", &highthreshold);

    Image result_highpassfilter = applyHighPassFilter(img1, highfilterType, highthreshold);
    // Vérification de l'écriture du fichier
if (writePGM("Assets/result_highfiltered.pgm", result_highpassfilter) == 0) {
    printf("Le filtrage passe-haut et l'écriture de l'image ont été effectués avec succès.\n");
} else {
    fprintf(stderr, "Erreur lors de l'écriture de l'image filtrée.\n");
}

//Additionnons les images
    Image result = addImages(img1, img2);
    writePGM("Assets/result.pgm", result);

    // Libérer la mémoire
    free(img1.data);
    free(img2.data);
    free(result.data);
    free(resultcontrast.data);
    free(result_lowpassfilter.data);
    free(result_highpassfilter.data);

    //printf("L'addition des images a été réalisée avec succès!\n");

    return 0;
}