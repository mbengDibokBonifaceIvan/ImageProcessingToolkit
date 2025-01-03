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


int main() {
    // Lire les images PGM
    Image img1 = readPGM("Assets/lena.512.pgm");
    Image img2 = readPGM("Assets/cristaux.512.pgm");
    Image img = readPGM("Assets/lena.512.pgm");
    int brightness_adjustment = 50; // Vous pouvez ajuster la luminosité en changeant cette valeur
    Image img_seuillage = readPGM("Assets/lena.512.pgm");
    int threshold_value = 128; // Vous pouvez ajuster le seuil en changeant cette valeur
    Image img_zoom_in = readPGM("Assets/lena.512.pgm");
    int newWidth = img.width * 2; // Zoom in (double la largeur)
    int newHeight = img.height * 2; // Zoom in (double la hauteur)
    Image img_zoomout = readPGM("Assets/lena.512.pgm");
    int newWidth2= img.width / 2; // Zoom out (divise la largeur par 2)
    int newHeight2 = img.height / 2; // Zoom out (divise la hauteur par 2)
    Image img_houg = readPGM("Assets/pentagone.1024.pgm"); // Supposons que vous avez une image binaire d'arêtes
    Image accumulator;
    Image img_niveau_de_gris = readPGM("Assets/boat.512.pgm"); // Charger une image en niveaux de gris
    int threshold = otsuThreshold(img_niveau_de_gris); // Trouver le seuil optimal avec la méthode d'Otsu




    // Vérifier que les images ont la même taille
    if (img1.width != img2.width || img1.height != img2.height) {
        fprintf(stderr, "Les images doivent avoir la même taille!\n");
        exit(EXIT_FAILURE);
    }

    // Addition des images
    Image result_add = addImages(img1, img2);
    writePGM("Resultats/result_addition.pgm", result_add);

    // Soustraction des images
    Image result_subtract = subtractImages(img1, img2);
    writePGM("Resultats/result_subtraction.pgm", result_subtract);

    // Ajustement de la luminosité
    Image result_brightness = adjustBrightness(img, brightness_adjustment);
    writePGM("Resultats/result_brightness.pgm", result_brightness);

    // Seuillage de l'image
     Image result_threshold = thresholdImage(img_seuillage, threshold_value);
    writePGM("Resultats/result_threshold.pgm", result_threshold);

    // Redimensionnement de l'image
    Image result_zoomin = resizeImage(img, newWidth, newHeight);
    writePGM("Resultats/result_zoom_in.pgm", result_zoomin);
    Image result_zoomout = resizeImage(img, newWidth2, newHeight2);
    writePGM("Resultats/result_zoom_out.pgm", result_zoomout);

    // Transformée de Hough
    houghTransform(img_houg, &accumulator);
    writePGM("Resultats/result_hough.pgm", accumulator);

    // Seuillage d'une image en niveaux de gris
    Image result_threshold2 = binarizeImage(img_niveau_de_gris, threshold);
    writePGM("Resultats/result_binarized.pgm", result_threshold2);


    // Libérer la mémoire
    free(img1.data);
    free(img2.data);
    free(result_add.data);
    free(result_subtract.data);
    free(img.data);
    free(result_brightness.data);
    free(img_seuillage.data);
    free(result_threshold.data);
    free(img_zoom_in.data);
    free(result_zoomin.data);
    free(img_zoomout.data);
    free(result_zoomout.data);
    free(img_houg.data);
    free(accumulator.data);
    free(img_niveau_de_gris.data);
    free(result_threshold2.data);



    printf("Le traitement des images a été réalisé avec succès!\n");

    return 0;
}