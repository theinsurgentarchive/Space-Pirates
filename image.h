#pragma once

#include <memory>

struct Image {
	uint16_t width, height;
	std::unique_ptr<unsigned char[]> data;
	~Image();
	Image(const std::string& file_name);
};
// class Image {
// 	public:
// 		uint16_t width, height;
// 		unsigned char *data;
// 		~Image() { delete [] data; }
// 		Image(const char *fname) { 
// 			if (fname[0] == '\0')
// 				return;
// 			int ppmFlag = 0;
// 			char name[40];
// 			strcpy(name, fname);
// 			int slen = strlen(name);
// 			char ppmname[80];
// 			if (strncmp(name+(slen-4), ".ppm", 4) == 0)
// 				ppmFlag = 1;
// 			if (ppmFlag) {
// 				strcpy(ppmname, name);
// 			} else {
// 				name[slen-4] = '\0';
// 				sprintf(ppmname,"%s.ppm", name);
// 				char ts[100];
// 				sprintf(ts, "convert %s %s", fname, ppmname);
// 				system(ts);
// 			}
// 			FILE *fpi = fopen(ppmname, "r");
// 			if (fpi) {
// 				char line[200];
// 				fgets(line, 200, fpi);
// 				fgets(line, 200, fpi);
// 				while (line[0] == '#' || strlen(line) < 2)
// 					fgets(line, 200, fpi);
// 				sscanf(line, "%hi %hi", &width, &height);
// 				fgets(line, 200, fpi);
// 				int n = width * height * 3;			
// 				data = new unsigned char[n];			
// 				for (int i=0; i<n; i++)
// 					data[i] = fgetc(fpi); 
// 				fclose(fpi);
// 			} else {
// 				DERRORF("ERROR opening image: %s\n",ppmname);
// 				exit(0);
// 			}
// 			if (!ppmFlag)
// 				unlink(ppmname);
// 		}
// };
//end - gordon