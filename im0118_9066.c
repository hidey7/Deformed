#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define  N 1024
unsigned char R[N][N], G[N][N], B[N][N];
unsigned char R0[N][N], G0[N][N], B0[N][N];
unsigned char bR[N][N], bG[N][N], bB[N][N];
unsigned char cR[N][N], cG[N][N], cB[N][N];

#include "myimini.h"

int label[N][N];
int  width, height;
void region_decomposition(int);
void labeling(int, int, int, int, int, int, int);
void edge_extraction_filter(void);
void tone_curve_gamma(char, float);
void allCopy(void);
void partialCopy(void);
void alpha_brending(void);
void split_inversion(void);
//void mask(void);
void negative(void);


int main(void)
{
      BMPfileHeader fileh;
      BMPinfoHeader infoh;

      readBMP("myface.bmp", &width, &height, &fileh, &infoh);
	  allCopy();
	  partialCopy();
      printf("Before\n");
      histogram(width, height);
      printf("\n");

	  
	  
      /******* transformation *******/
	  
	  tone_curve_gamma('r', 3.0);
	  tone_curve_gamma('g', 1.0);
	  tone_curve_gamma('b', 3.0);
      region_decomposition(40);   /* dPVの値 (dPV < 35) */
	  
	  split_inversion();
	  negative();
	  //alpha_brending();
	  //edge_extraction_filter();


      /******* transformation *******/

      printf("After\n");
      histogram(width, height);
      writeBMP("outputimage.bmp", &width, &height, &fileh, &infoh);

      return 0;

}

void negative(void){
	int i, j;
	
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			R[i][j] = 255.0 - R[i][j];
			G[i][j] = 255.0 - G[i][j];
			B[i][j] = 255.0 - B[i][j];
		}
	}
}

/*
void mask(void){
	int i, j;
	for(i = 0; i < height; i++){
		for(j = 0; j < width / 2; j++){
			if(cR[i][j] + cG[i][j] + cB[i][j] > 445 && cR[i][j] + cG[i][j] + cB[i][j] < 450){
				cR[i][j] = cG[i][j] = cB[i][j] = 0;
			}else{
				cR[i][j] = cG[i][j] = cB[i][j] = 255;
			}
		}
	}
	
}
*/


void split_inversion(void){
	int i, j;
	for(i = 0; i < height; i++){
		for(j = width / 2; j < width; j++){
			R[i][j - (width / 2)] = R[i][j];
			R[i][j] = cR[i][j - (width / 2)];
			G[i][j - (width / 2)] = G[i][j];
			G[i][j] = cG[i][j - (width / 2)];
			B[i][j - (width / 2)] = B[i][j];
			B[i][j] = cB[i][j - (width / 2)];
		}
	}
}

void alpha_brending(void) {
	int i, j;
	double alpha;
	
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			alpha = 1 / width * j;
			R[i][j] = alpha * bR[i][j] + (1 - alpha) * R[i][j];
			G[i][j] = alpha * bG[i][j] + (1 - alpha) * G[i][j];
			B[i][j] = alpha * bB[i][j] + (1 - alpha) * B[i][j];
		}
	}
}

void allCopy(void){
	int i, j;
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			bR[i][j] = R[i][j];
			bG[i][j] = G[i][j];
			bB[i][j] = B[i][j];
		}
	}
}

void partialCopy(void) {
	int i, j;
	
	for(i = 0; i < height; i++){
		for(j = 0; j < width / 2; j++){
			cR[i][j] = R[i][j];
			cG[i][j] = G[i][j];
			cB[i][j] = B[i][j];
		}
	}
}

void edge_extraction_filter(void){

	int i, j;
	int r0, g0, b0;
	int rx, gx, bx;
	int ry, gy, by;

	for(i = 0; i < height; i++){
	
		for(j = 0; j < width; j++){
			R0[i][j] = R[i][j];
			G0[i][j] = G[i][j];
			B0[i][j] = B[i][j];
		}
	}

	for(i = 1; i < height - 1; i++){
		for(j = 1; j < width - 1; j++){

			rx = R0[i][j + 1] - R0[i][j - 1];
			gx = G0[i][j + 1] - G0[i][j - 1];
			bx = B0[i][j + 1] - B0[i][j - 1];

			ry = R0[i + 1][j] - R0[i - 1][j];
			gy = G0[i + 1][j] - G0[i - 1][j];
			by = B0[i + 1][j] - B0[i - 1][j];

			r0 = sqrt(rx*rx + ry*ry);
			g0 = sqrt(gx*gx + gy*gy);
			b0 = sqrt(bx*bx + by*by);

			if(r0 > 255) r0 = 255;
			if(g0 > 255) g0 = 255;
			if(b0 > 255) b0 = 255;
			
			R[i][j] = r0;
			G[i][j] = g0;
			B[i][j] = b0;

		}
	}
}

void region_decomposition(int dPV){

	int R0, G0, B0, i0, j0, i, j, k, LMAX, LABEL=1;
	int *area, *Rsum, *Gsum, *Bsum;
	
	for(i = 0; i < height; i++)
		for(j = 0; j < width; j++)
			label[i][j] = 0;

	/* ラベル貼り */
	for(i = 0; i < height; i++)
		for(j = 0; j < width; j++)
			if(!label[i][j]) {
				i0 = i; j0 = j;
				label[i0][j0] = LABEL;
				R0 = R[i0][j0];
				G0 = G[i0][j0];
				B0 = B[i0][j0];
				labeling(R0, G0, B0, i0, j0, LABEL, dPV);
				LABEL++;
			}

	LMAX = LABEL - 1;
	area = (int *)(malloc(sizeof(int)*(LMAX+1)));
	Rsum = (int *)(malloc(sizeof(int)*(LMAX+1)));
	Gsum = (int *)(malloc(sizeof(int)*(LMAX+1)));
	Bsum = (int *)(malloc(sizeof(int)*(LMAX+1)));

	for(k = 1; k <= LMAX; k++){
		area[k] = 0;
		Rsum[k] = 0;
		Gsum[k] = 0;
		Bsum[k] = 0;
	}

	for(i = 0; i < height; i++)
		for(j = 0; j < width; j++){
			area[ label[i][j] ]++;
			Rsum[ label[i][j] ] += R[i][j];
			Gsum[ label[i][j] ] += G[i][j];
			Bsum[ label[i][j] ] += B[i][j];
		}

	for(i = 0; i < height; i++)
		for(j = 0; j < width; j++){
			R[i][j] = Rsum[ label[i][j] ] / area[ label[i][j] ];
			G[i][j] = Gsum[ label[i][j] ] / area[ label[i][j] ];
			B[i][j] = Bsum[ label[i][j] ] / area[ label[i][j] ];
		}

}

void labeling(int R0, int G0, int B0, int i0, int j0, int LABEL, int dPV){
	
	int i, j;

	for(i = i0 - 1; i <= i0 + 1; i++)
		for(j = j0 - 1; j <= j0 + 1; j++)
			if(!(i == i0 && j == j0) && i >= 0 && i < height && j >= 0 && j < width)

				/* (2.1)の処理 */
				if(label[i][j] == 0 && abs(R[i][j] - R0) <= dPV && abs(G[i][j] - G0) <= dPV && abs(B[i][j] - B0) <= dPV) {
					/* 画素(i, j)にLABELを貼り付ける */
					/* 再帰の処理が必要 */

					label[i][j] = LABEL;

					labeling(R0, G0, B0, i, j, LABEL, dPV);
				}

}

void tone_curve_gamma(char rgb, float gamma){

	int i, j;
	unsigned char r0, g0, b0;

	for(i = 0; i < height; i++){
	
		for(j = 0; j < width; j++){
			switch(rgb){
				case 'r':
					r0 = R[i][j];
					R[i][j] = 255.0 * pow(r0 / 255.0, 1.0 / gamma); 
					break;
				case 'g':
					g0 = G[i][j];
					G[i][j] = 255.0 * pow(g0 / 255.0, 1.0 / gamma);
					break;
				case 'b':
					b0 = B[i][j];
					B[i][j] = 255.0 * pow(b0 / 255.0, 1.0 / gamma);
					break;

			}
		}


	}
}
