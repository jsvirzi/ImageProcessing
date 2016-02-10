#include "entropy.h"

using namespace cv;

enum {
	DisplayAll,
	DisplayY,
	DisplayU,
	DisplayV,
	DisplayB,
	DisplayG,
	DisplayR
};

int main(int argc, char ** argv)
{

	int i, j, which = DisplayY;
	std::string ifile = "lena.jpg";
	bool debug = false;

	for(i=1;i<argc;++i) {
		if(strcmp(argv[i], "-debug") == 0) debug = true;
		else if(strcmp(argv[i], "-i") == 0) ifile = argv[++i]; 
		else if(strcmp(argv[i], "-all") == 0) which = DisplayAll; 
		else if(strcmp(argv[i], "-y") == 0) which = DisplayY; 
		else if(strcmp(argv[i], "-u") == 0) which = DisplayU; 
		else if(strcmp(argv[i], "-v") == 0) which = DisplayV; 
		else if(strcmp(argv[i], "-b") == 0) which = DisplayB; 
		else if(strcmp(argv[i], "-g") == 0) which = DisplayG; 
		else if(strcmp(argv[i], "-r") == 0) which = DisplayR; 
		else { printf("unrecognized argument [%s]\n", argv[i]); return 1; }
	}

	const char *filename = ifile.c_str(); 
	VideoCapture cap(filename); /* open input stream - camera or file */ 
	const char *windowName = "main";
	namedWindow(windowName, WINDOW_AUTOSIZE);

	int row, rows = 360, col, cols = 640, b, g, r, nTilesX = 8, nTilesY = 8, frame_index = 0;
	double global_intensity_histogram[256];
	float y, u, v;

	int tile_rows = rows / nTilesY; /* how many pixels represent one tile from the original image */
	int tile_cols = cols / nTilesX; /* how many pixels represent one tile from the original image */
	Mat **tiles_y = new Mat * [ nTilesY ];
	Mat **tiles_u = new Mat * [ nTilesY ];
	Mat **tiles_v = new Mat * [ nTilesY ];
	Mat **tiles_b = new Mat * [ nTilesY ];
	Mat **tiles_g =_ new Mat * [ nTilesY ];
	Mat **tiles_r = new Mat * [ nTilesY ];
	double **mean_y = new double * [ nTilesY ];
	double **mean_u = new double * [ nTilesY ];
	double **mean_v = new double * [ nTilesY ];
	double **mean_b = new double * [ nTilesY ];
	double **mean_g = new double * [ nTilesY ];
	double **mean_r = new double * [ nTilesY ];
	double **entropy_y = new double * [ nTilesY ];
	double **entropy_u = new double * [ nTilesY ];
	double **entropy_v = new double * [ nTilesY ];
	double **entropy_b = new double * [ nTilesY ];
	double **entropy_g = new double * [ nTilesY ];
	double **entropy_r = new double * [ nTilesY ];
	for(j=0;j<nTilesY;++j) { 
		tiles_y[j] = new Mat [ nTilesX ]; 
		tiles_u[j] = new Mat [ nTilesX ]; 
		tiles_v[j] = new Mat [ nTilesX ]; 
		tiles_b[j] = new Mat [ nTilesX ]; 
		tiles_g[j] = new Mat [ nTilesX ]; 
		tiles_r[j] = new Mat [ nTilesX ]; 
		mean_y[j] = new double [ nTilesX ];
		mean_u[j] = new double [ nTilesX ];
		mean_v[j] = new double [ nTilesX ];
		mean_b[j] = new double [ nTilesX ];
		mean_g[j] = new double [ nTilesX ];
		mean_r[j] = new double [ nTilesX ];
		entropy_y[j] = new double [ nTilesX ];
		entropy_u[j] = new double [ nTilesX ];
		entropy_v[j] = new double [ nTilesX ];
		entropy_b[j] = new double [ nTilesX ];
		entropy_g[j] = new double [ nTilesX ];
		entropy_r[j] = new double [ nTilesX ];
		for(i=0;i<nTilesX;++i) {
			tiles_y[j][i] = Mat::zeros(tile_rows, tile_cols, CV_32F);
			tiles_u[j][i] = Mat::zeros(tile_rows, tile_cols, CV_32F);
			tiles_v[j][i] = Mat::zeros(tile_rows, tile_cols, CV_32F);
			tiles_b[j][i] = Mat::zeros(tile_rows, tile_cols, CV_32F);
			tiles_g[j][i] = Mat::zeros(tile_rows, tile_cols, CV_32F);
			tiles_r[j][i] = Mat::zeros(tile_rows, tile_cols, CV_32F);
		}
	}

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
				float *ybuff = &tiles_y[tile_row_index][tile_col_index];
				float *ubuff = &tiles_y[tile_row_index][tile_col_index];
				float *vbuff = &tiles_y[tile_row_index][tile_col_index];
				float *bbuff = &tiles_y[tile_row_index][tile_col_index];
				float *gbuff = &tiles_y[tile_row_index][tile_col_index];
				float *rbuff = &tiles_y[tile_row_index][tile_col_index];
				b = cbuff[thumbnail.step * row + col * 3 + 0];
				g = cbuff[thumbnail.step * row + col * 3 + 1];
				r = cbuff[thumbnail.step * row + col * 3 + 2];
				int tile_col = col % tile_cols;
				switch(which) {
				case DisplayAll:
				case DisplayY:
					y =  0.299 * r / 256.0 +  0.587 * g / 256.0 +  0.114 * b / 256.0;
					if(y < 0.0) y = 0.0; if(y > 1.0) y = 1.0;
					ybuff[tile_cols * tile_row + tile_col] = y;
					if(which != DisplayAll) break;
				case DisplayU:
					u = -0.147 * r / 256.0 + -0.289 * g / 256.0 +  0.436 * b / 256.0;
					if(u < 0.0) u = 0.0; if(u > 1.0) u = 1.0;
					ubuff[tile_cols * tile_row + tile_col] = u;
					if(which != DisplayAll) break;
				case DisplayV:
					v =  0.615 * r / 256.0 + -0.515 * g / 256.0 + -0.100 * b / 256.0;
					if(v < 0.0) v = 0.0; if(v > 1.0) v = 1.0;
					vbuff[tile_cols * tile_row + tile_col] = v;
					if(which != DisplayAll) break;
				case DisplayB:
					bbuff[tile_cols * tile_row + tile_col] = b / 255.0;
					if(which != DisplayAll) break;
				case DisplayG:
					gbuff[tile_cols * tile_row + tile_col] = g / 255.0;
					if(which != DisplayAll) break;
				case DisplayR:
					rbuff[tile_cols * tile_row + tile_col] = r / 255.0;
					if(which != DisplayAll) break;
				default:
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

		Mat tiled_mat = Mat::zeros(rows, cols, CV_32F);
		for(i=0;i<nTilesX;++i) {
			for(j=0;j<nTilesY;++j) { 
				Mat q(tiled_mat, Rect(i * tile_cols, j * tile_rows, tile_cols, tile_rows));
				tiles[j][i].copyTo(q);
				mean_intensity[i][j] = mean_image_intensity(tiles[i][j]);
				tile_entropy[i][j] = image_entropy(tiles[i][j], global_intensity_histogram);
				printf("tile(%d,%d) intensity = %f\n", i, j, mean_intensity[i][j]); 
			}
		}

		int separation_width = 10;
		tile_cols = tile_rows = 32; /* now tiles refer to the Y U V and B G R tiles */
		Mat entropy_mat = Mat::zeros(3 * tile_rows + separation_width, 2 * tile_cols + separation_width, CV_32F);
		for(i=0;i<nTilesX;++i) {
			for(j=0;j<nTilesY;++j) { 
				Mat q_y(entropy_mat, Rect(0 * tile_cols, 0 * tile_rows, tile_cols, tile_rows));
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

		imshow(windowName, tiled_mat);
		imshow("generic", intensity_frame);
		imshow("entropy", entropy_frame);
		imshow("image", thumbnail);

		waitKey(30);

	}

	return 0;
}
