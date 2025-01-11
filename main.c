#include <stdio.h>
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

// Fonction pour écrire une image PGM
void writePGM(const char *filename, Image img) {
    FILE *file = fopen(filename, "w");
    fprintf(file, "P5\n%d %d\n255\n", img.width, img.height);
    fwrite(img.data, sizeof(unsigned char), img.width * img.height, file);
    fclose(file);
}


// Fonction pour écrire une image PGM par Amirah
int writePGM2(const char *filename, Image img) {
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

// Fonction pour soustraire deux images
Image subtractImages(Image img1, Image img2) {
    Image result;
    result.width = img1.width;
    result.height = img1.height;
    result.data = (unsigned char *)malloc(result.width * result.height);

    for (int i = 0; i < result.width * result.height; i++) {
        int diff = img1.data[i] - img2.data[i];
        result.data[i] = (diff < 0) ? 0 : diff; // Clamping à 0
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

// Fonction pour modifier la luminosité d'une image
Image adjustBrightness(Image img, int adjustment) {
    Image result;
    result.width = img.width;
    result.height = img.height;
    result.data = (unsigned char *)malloc(result.width * result.height);

    for (int i = 0; i < result.width * result.height; i++) {
        int new_val = img.data[i] + adjustment;
        result.data[i] = (new_val > 255) ? 255 : (new_val < 0) ? 0 : new_val; // Clamping entre 0 et 255
    }

    return result;
}

// Fonction pour seuiller une image
Image thresholdImage(Image img, int threshold) {
    Image result;
    result.width = img.width;
    result.height = img.height;
    result.data = (unsigned char *)malloc(result.width * result.height);

    for (int i = 0; i < result.width * result.height; i++) {
        result.data[i] = (img.data[i] < threshold) ? 0 : 255; // Seuillage binaire
    }

    return result;
}


// Fonction pour redimensionner une image (zoom in ou zoom out)
Image resizeImage(Image img, int newWidth, int newHeight) {
    Image result;
    result.width = newWidth;
    result.height = newHeight;
    result.data = (unsigned char *)malloc(result.width * result.height);

    // Calcul des ratios de redimensionnement
    float x_ratio = (float)(img.width - 1) / result.width;
    float y_ratio = (float)(img.height - 1) / result.height;

    for (int i = 0; i < result.height; i++) {
        for (int j = 0; j < result.width; j++) {
            int x = (int)(x_ratio * j);
            int y = (int)(y_ratio * i);
            int x_diff = (x_ratio * j) - x;
            int y_diff = (y_ratio * i) - y;
            int index = y * img.width + x;

            // Interpolation bilinéaire
            unsigned char pixel = (1 - x_diff) * (1 - y_diff) * img.data[index] +
                                  x_diff * (1 - y_diff) * img.data[index + 1] +
                                  (1 - x_diff) * y_diff * img.data[index + img.width] +
                                  x_diff * y_diff * img.data[index + img.width + 1];

            result.data[i * result.width + j] = pixel;
        }
    }

    return result;
}

// Fonction pour appliquer la transformée de Hough
void houghTransform(Image img, Image *accumulator) {
    int theta_max = 180; // Nombre de pas pour les angles (180 degrés)
    int rho_max = (int)sqrt(img.width * img.width + img.height * img.height); // Longueur maximale du vecteur rho
    accumulator->width = theta_max;
    accumulator->height = 2 * rho_max + 1;
    accumulator->data = (unsigned char *)calloc(accumulator->width * accumulator->height, sizeof(unsigned char));

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            if (img.data[y * img.width + x] > 0) {
                for (int theta = 0; theta < theta_max; theta++) {
                    double rad = theta * M_PI / 180.0;
                    double rho = x * cos(rad) + y * sin(rad);
                    int rho_index = (int)round(rho) + rho_max;
                    accumulator->data[rho_index * accumulator->width + theta]++;
                }
            }
        }
    }
}

// Fonction pour calculer l'histogramme d'une image en niveaux de gris
void computeHistogram(Image img, int *histogram) {
    for (int i = 0; i < img.width * img.height; i++) {
        histogram[img.data[i]]++;
    }
}

// Fonction pour calculer la méthode d'Otsu pour trouver le seuil optimal
int otsuThreshold(Image img) {
    int histogram[256] = {0};
    computeHistogram(img, histogram);

    int total = img.width * img.height;
    double sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += i * histogram[i];
    }

    double sumB = 0;
    int wB = 0;
    int wF;
    double varMax = 0;
    int threshold = 0;

    for (int i = 0; i < 256; i++) {
        wB += histogram[i];
        if (wB == 0) continue;
        wF = total - wB;
        if (wF == 0) break;

        sumB += i * histogram[i];
        double mB = sumB / wB;
        double mF = (sum - sumB) / wF;

        double varBetween = wB * wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = i;
        }
    }

    return threshold;
}

// Fonction pour binariser une image en niveaux de gris en utilisant un seuil
Image binarizeImage(Image img, int threshold) {
    Image result;
    result.width = img.width;
    result.height = img.height;
    result.data = (unsigned char *)malloc(result.width * result.height);

    for (int i = 0; i < result.width * result.height; i++) {
        result.data[i] = (img.data[i] > threshold) ? 255 : 0;
    }

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

//Menu
void showMenu(void) {
    printf("\n=== MENU DE TRAITEMENT D'IMAGES ===\n");
    printf("1. Addition d'images\n");
    printf("2. Soustraction d'images\n");
    printf("3. Modification du contraste\n");
    printf("4. Filtre passe-bas\n");
    printf("5. Filtre passe-haut\n");
    printf("6. Ajustement de la luminosité\n");
    printf("7. Seuillage\n");
    printf("8. Redimensionnement(Zoom avant)\n");
    printf("9. Redimensionnement(Zoom arrière)\n");
    printf("10. Transformée de Hough\n");
    printf("11. Binarisation (méthode d'Otsu)\n");
    printf("0. Quitter\n");
}

// Fonction principale de traitement
void processImages(void) {

    // Lire les images PGM
    Image img1 = readPGM("Assets/lena.512.pgm");
    Image img2 = readPGM("Assets/cristaux.512.pgm");
    Image img = readPGM("Assets/lena.512.pgm");

    // Vérifier que les images ont la même taille
    if (img1.width != img2.width || img1.height != img2.height) {
        fprintf(stderr, "Les images doivent avoir la même taille!\n");
        exit(EXIT_FAILURE);
    }

    int choice;
   
    do {
        showMenu();
        printf("\nChoisissez une option (0-10) : ");
        scanf("%d", &choice);

        if (choice == 0) break;

        switch(choice) {
            case 1: {
                // Addition d'images
                Image result = addImages(img1, img2);
                writePGM("Resultats/resultat_addition.pgm", result);
                free(result.data);
                break;
            }
            case 2: {
                // Soustraction d'images
                Image result = subtractImages(img1, img2);
                writePGM("Resultats/resultat_soustraction.pgm", result);
                free(result.data);
                break;
            }
            case 3: {
                // Modification du contraste
                float contrast = 1.5; // Vous pouvez ajuster le contraste en changeant cette valeur
                Image result = modifyContrast(img, contrast);
                writePGM("Resultats/resultat_contraste.pgm", result);
                free(result.data);
                break;
            }
            case 4: {
                // Filtre passe-bas
                int filterType, filterSize;
                printf("Choisissez le type de filtre (1: Moyenneur, 2: Gaussien) : ");
                scanf("%d", &filterType);
                printf("Entrez la taille du filtre ou la variance (selon le type) : ");
                scanf("%d", &filterSize);

                Image result = applyLowPassFilter(img1, filterType, filterSize);
                writePGM("Resultats/resultat_filtre_bas.pgm", result);
                free(result.data);
                break;
            }
            case 5: {
                // Filtre passe-haut
                int highfilterType;
                float highthreshold;
                printf("Choisissez le type de filtre (1: Prewitt, 2: Roberts, 3: Laplacien) : ");
                scanf("%d", &highfilterType);
                printf("Entrez la valeur du seuil (choisir entre 1 et 10): ");
                scanf("%f", &highthreshold);

                Image result = applyHighPassFilter(img1, highfilterType, highthreshold);
                writePGM("Resultats/resultat_filtre_haut.pgm", result);
                free(result.data);
                break;
            }
            case 6: {
                // Ajustement de la luminosité
                int brightness_adjustment = 50; // Vous pouvez ajuster la luminosité en changeant cette valeur
                Image result = adjustBrightness(img, brightness_adjustment);
                writePGM("Resultats/result_brightness.pgm", result);
                free(result.data);
                break;
            }
            case 7: {
                // Seuillage de l'image
                int threshold_value = 128; // Vous pouvez ajuster le seuil en changeant cette valeur
                Image result = thresholdImage(img, threshold_value);
                writePGM("Resultats/result_threshold.pgm", result);
                free(result.data);
                break;
            }
            case 8: {
                // Redimensionnement de l'image
                int newWidth = img.width * 2; // Zoom in (double la largeur)
                int newHeight = img.height * 2; // Zoom in (double la hauteur)
                Image result = resizeImage(img, newWidth, newHeight);
                writePGM("Resultats/result_zoom_in.pgm", result);
                free(result.data);
                break;
            }
            case 9: {
                // Redimensionnement de l'image
                int newWidth = img.width / 2; // Zoom out (moitié la largeur)
                int newHeight = img.height / 2; // Zoom out (moitié la hauteur)
                Image result = resizeImage(img, newWidth, newHeight);
                writePGM("Resultats/result_zoom_out.pgm", result);
                free(result.data);
                break;
            }
            case 10: {
                // Transformée de Hough
                Image accumulator;
                houghTransform(img, &accumulator);
                writePGM("Resultats/result_hough.pgm", accumulator);
                free(accumulator.data);
                break;
            }
            case 11: {
                // Binarisation (méthode d'Otsu)
                int threshold = otsuThreshold(img); // Trouver le seuil optimal avec la méthode d'Otsu
                Image result = thresholdImage(img, threshold);
                writePGM("Resultats/result_binarisation.pgm", result);
                free(result.data);
                break;
            }
            default:
                printf("Option invalide. Veuillez réessayer.\n");
                break;
                
        }

     printf("\nTraitement terminé avec succès !\n");
    } while(1);
}

int main() {
   processImages();
    return 0;
}

