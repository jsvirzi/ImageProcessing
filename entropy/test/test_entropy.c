#include "entropy.h"

using namespace cv;

enum {
	DisplayY,
	DisplayU,
	DisplayV,
	DisplayB,
	DisplayG,
	DisplayR
};

int main(int argc, char ** argv)
{

	int i, j, which = 0;
	std::string ifile = "lena.jpg";
	bool debug = false;

	for(i=1;i<argc;++i) {
		if(strcmp(argv[i], "-debug") == 0) debug = true;
		else if(strcmp(argv[i], "-i") == 0) ifile = argv[++i]; 
		else if(strcmp(argv[i], "-y") == 0) which = DisplayY; 
		else if(strcmp(argv[i], "-u") == 0) which = DisplayU; 
		else if(strcmp(argv[i], "-v") == 0) which = DisplayV; 
		else if(strcmp(argv[i], "-b") == 0) which = DisplayB; 
		else if(strcmp(argv[i], "-g") == 0) which = DisplayG; 
		else if(strcmp(argv[i], "-r") == 0) which = DisplayR; 
		else { printf("unrecognized argument [%s]\n", argv[i]); return 1; }
	}

	const char* filename = ifile.c_str(); 
	VideoCapture cap(filename); /* open input stream - camera or file */ 
	const char *windowName = "main";
	namedWindow(windowName, WINDOW_AUTOSIZE);

	int row, rows = 360, col, cols = 640, b, g, r, nTilesX = 8, nTilesY = 8;

	int tile_rows = rows / nTilesY;
	int tile_cols = cols / nTilesX;
	Mat **tiles = new Mat * [ nTilesY ];
	double **mean_intensity = new double * [ nTilesY ];
	double **tile_entropy = new double * [ nTilesY ];
	for(j=0;j<nTilesY;++j) { 
		tiles[j] = new Mat [ nTilesX ]; 
		mean_intensity[j] = new double [ nTilesX ];
		tile_entropy[j] = new double [ nTilesX ];
		for(i=0;i<nTilesX;++i) {
			tiles[j][i] = Mat::zeros(tile_rows, tile_cols, CV_32F);
		}
	}

	int frame_index = 0;

	double global_intensity_histogram[256];
	float y, u, v;
	while(true) {
		Mat frame, thumbnail;
		cap >> frame;

		Size size(cols, rows);
		resize(frame, thumbnail, size);

		unsigned char *cbuff = (unsigned char *)thumbnail.data;

		printf("frame index = %d\n", frame_index);
		++frame_index;

/* convert byte BGR to float YUV */
		for(row=0;row<rows;++row) {
			int tile_row_index = row / tile_rows;
			int tile_row = row % tile_rows;
			for(col=0;col<cols;++col) {
				int tile_col_index = col / tile_cols;
				Mat *tile = &tiles[tile_row_index][tile_col_index];
				float *dbuff = (float *)tile->data;
				b = cbuff[thumbnail.step * row + col * 3 + 0];
				g = cbuff[thumbnail.step * row + col * 3 + 1];
				r = cbuff[thumbnail.step * row + col * 3 + 2];
				int tile_col = col % tile_cols;
				switch(which) {
				case DisplayY:
					y =  0.299 * r / 256.0 +  0.587 * g / 256.0 +  0.114 * b / 256.0;
					if(y < 0.0) y = 0.0; if(y > 1.0) y = 1.0;
					dbuff[tile_cols * tile_row + tile_col] = y;
					break;
				case DisplayU:
					u = -0.147 * r / 256.0 + -0.289 * g / 256.0 +  0.436 * b / 256.0;
					if(u < 0.0) u = 0.0; if(u > 1.0) u = 1.0;
					dbuff[tile_cols * tile_row + tile_col] = u;
					break;
				case DisplayV:
					v =  0.615 * r / 256.0 + -0.515 * g / 256.0 + -0.100 * b / 256.0;
					if(v < 0.0) v = 0.0; if(v > 1.0) v = 1.0;
					dbuff[tile_cols * tile_row + tile_col] = v;
					break;
				case DisplayB:
					dbuff[tile_cols * tile_row + tile_col] = b / 255.0;
					break;
				case DisplayG:
					dbuff[tile_cols * tile_row + tile_col] = g / 255.0;
					break;
				case DisplayR:
					dbuff[tile_cols * tile_row + tile_col] = r / 255.0;
					break;
				}
			}
		}

	// tiles[1][0] = Mat::zeros(tile_cols, tile_rows, CV_32F);

		Mat gray_thumbnail;
		cvtColor(thumbnail, gray_thumbnail, CV_BGR2GRAY);
		gray_thumbnail.convertTo(gray_thumbnail, CV_32F);
		normalize(gray_thumbnail, gray_thumbnail, 0.0, 1.0, CV_MINMAX);

		if(frame_index < 10) {
			make_pdf(gray_thumbnail, global_intensity_histogram);
		}

		Mat bigMat = Mat::zeros(rows, cols, CV_32F);
		for(i=0;i<nTilesX;++i) {
			for(j=0;j<nTilesY;++j) { 
				Mat q(bigMat, Rect(i * tile_cols, j * tile_rows, tile_cols, tile_rows));
				tiles[j][i].copyTo(q);
				mean_intensity[i][j] = mean_image_intensity(tiles[i][j]);
				tile_entropy[i][j] = image_entropy(tiles[i][j], global_intensity_histogram);
				printf("tile(%d,%d) intensity = %f\n", i, j, mean_intensity[i][j]); 
			}
		}

		Mat intensity_frame = Mat::zeros(nTilesX * 32, nTilesY * 32, CV_32F);
		Mat entropy_frame = Mat::zeros(nTilesX * 32, nTilesY * 32, CV_32F);
		float *pi = (float *)intensity_frame.data;
		float *pe = (float *)entropy_frame.data;
		for(i=0;i<nTilesX;++i) {
			for(j=0;j<nTilesY;++j) {
			   double intensity = mean_intensity[i][j];
			   double entropy = tile_entropy[i][j];
			   for(int ii=0;ii<32;++ii) {
				   col = i * 32 + ii;
				   for(int jj=0;jj<32;++jj) {
					   row = j * 32 + jj;
					   pi[col * 32 * nTilesY + row] = intensity; /* reversed */
					   pe[col * 32 * nTilesY + row] = entropy; /* reversed */
				   }
			   }
			}
		}


entropy_frame = 1.0 / 175.0 * entropy_frame;

		double entropy_min = 0.0, entropy_max = 0.0;
		minMaxLoc(entropy_frame, &entropy_min, &entropy_max);
		printf("entropy min/max = %f,%f\n", entropy_min, entropy_max);
//		normalize(entropy_frame, entropy_frame, 0, 1, CV_MINMAX);

		imshow(windowName, bigMat);
		imshow("generic", intensity_frame);
		imshow("entropy", entropy_frame);
		imshow("image", thumbnail);

		waitKey(30);

	}

	return 0;
}
