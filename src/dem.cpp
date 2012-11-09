#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "gdalwarper.h"
#include "cpl_vsi.h"
#include "cpl_conv.h"
#include "cpl_string.h"

struct deminfo {
  char mesh[10];
  double W, E, S, N;
  int lowx, lowy, highx, highy, startx, starty;
  float *alti;
};

char *cut (const char *str, const char *ss, const char *es, char *buff)
{
  char *wk, *s, *e, *c;

  wk = strdup (str);
  if (wk == NULL) {
	free (wk);
	return NULL;
  }
  s = strstr (wk, ss);
  if (s == NULL) {
	free (wk);
	return NULL;
  }
  c = s + strlen (ss);
  e = strstr (c, es);
  if (e == NULL) {
	free (wk);
	return NULL;
  }
  *e = '\0';
  strcpy (buff, c);
  free (wk);
  return buff;
}

int
makeGeotiff (struct deminfo *d0, char *outpath,int nodata)
{

  GDALAllRegister ();

  GDALDataType band_type = GDT_Float32;
  int bands = 1;
  int dsn_xsize = (d0->highx - d0->lowx + 1);
  int dsn_ysize = (d0->highy - d0->lowy + 1);
  char **papszCreateOptions = NULL;
  papszCreateOptions = CSLSetNameValue (papszCreateOptions, "PROFILE", "GeoTIFF");
  //papszCreateOptions = CSLSetNameValue( papszCreateOptions, "TFW", "YES" );
  //papszCreateOptions = CSLSetNameValue (papszCreateOptions, "INTERLEAVE", "PIXEL");
  //papszCreateOptions = CSLSetNameValue (papszCreateOptions, "TILED", "YES");
  //papszCreateOptions = CSLSetNameValue (papszCreateOptions, "COMPRESS", "LZW");


  GDALDriverH hDriver = GDALGetDriverByName ("GTiff");
  GDALDatasetH hDsnDS = GDALCreate (hDriver, outpath, dsn_xsize, dsn_ysize, bands, band_type, papszCreateOptions);

  double dsnGeoTransform[6];
  dsnGeoTransform[0] = d0->W;
  dsnGeoTransform[1] = (d0->E - d0->W) / dsn_xsize;
  dsnGeoTransform[2] = 0;
  dsnGeoTransform[3] = d0->N;
  dsnGeoTransform[4] = 0;
  dsnGeoTransform[5] = -1.0 * (d0->N - d0->S) / dsn_ysize;
  GDALSetGeoTransform (hDsnDS, dsnGeoTransform);

  char pszSRS_WKT[1024] = "GEOGCS[\"JGD2000\", DATUM[\"Japanese Geodetic Datum 2000\", SPHEROID[\"GRS 1980\", 6378137.0, 298.257222101, AUTHORITY[\"EPSG\",\"7019\"]], TOWGS84[0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],AUTHORITY[\"EPSG\",\"6612\"]], PRIMEM[\"Greenwich\", 0.0, AUTHORITY[\"EPSG\",\"8901\"]], UNIT[\"degree\", 0.017453292519943295], AXIS[\"Geodetic longitude\", EAST], AXIS[\"Geodetic latitude\", NORTH], AUTHORITY[\"EPSG\",\"4612\"]]";
  GDALSetProjection (hDsnDS, pszSRS_WKT);
  GDALRasterBandH t_band = GDALGetRasterBand (hDsnDS, 1);
  if(nodata==1){
   GDALSetRasterNoDataValue (t_band, -9999);
  }
  GDALRasterIO (t_band, GF_Write, 0, 0, dsn_xsize, dsn_ysize, d0->alti, dsn_xsize, dsn_ysize, band_type, 0, 0);
  CSLDestroy (papszCreateOptions);
  GDALClose (hDsnDS);



  return 0;
}

int
main (int argc, char *argv[])
{

  struct deminfo d0;
  FILE *fp;
  char buf[256];
  char cbuf[256];
  int dnum, snum, k;
  int nodata;

  nodata = atoi(argv[2]);
  
  fp = fopen (argv[1], "r");
  printf ("start...\n");
  while (1) {
	fgets (buf, 256, fp);

	if (strstr (buf, "<mesh>") != NULL) {
	  strcpy (d0.mesh, cut (buf, "<mesh>", "</mesh>", cbuf));
	} else if (strstr (buf, "<gml:lowerCorner>") != NULL) {
	  sscanf (cut (buf, "<gml:lowerCorner>", "</gml:lowerCorner>", cbuf), "%lf %lf", &d0.S, &d0.W);
	} else if (strstr (buf, "<gml:upperCorner>") != NULL) {
	  sscanf (cut (buf, "<gml:upperCorner>", "</gml:upperCorner>", cbuf), "%lf %lf", &d0.N, &d0.E);
	} else if (strstr (buf, "<gml:low>") != NULL) {
	  sscanf (cut (buf, "<gml:low>", "</gml:low>", cbuf), "%d %d", &d0.lowx, &d0.lowy);
	} else if (strstr (buf, "<gml:high>") != NULL) {
	  sscanf (cut (buf, "<gml:high>", "</gml:high>", cbuf), "%d %d", &d0.highx, &d0.highy);;

	} else if (strstr (buf, "<gml:tupleList>") != NULL) {
	  dnum = (d0.highx - d0.lowx + 1) * (d0.highy - d0.lowy + 1);
	  d0.alti = (float *) malloc (sizeof (float) * (dnum+1));
	  printf ("mesh:%s\n", d0.mesh);
	  printf ("N:%lf,S:%lf,W:%lf,E:%lf\n", d0.N, d0.S, d0.W, d0.E);
	  printf ("col row: %d %d\n", d0.highx - d0.lowx + 1, d0.highy - d0.lowy + 1);
	  printf ("Reading data ...\n");

	  k = 0;
	  fgets (buf, 256, fp);
	  while (strstr (buf, ",") != NULL) {
	d0.alti[k++] = (float) atof (cut (buf, ",", "\n", cbuf));
	fgets (buf, 256, fp);
	  }
	} else if (strstr (buf, "<gml:startPoint>") != NULL) {
	  sscanf (cut (buf, "<gml:startPoint>", "</gml:startPoint>", cbuf), "%d %d", &d0.startx, &d0.starty);
	  snum = (d0.highx + 1) * d0.starty + d0.startx;

	}

	if (feof (fp)) {
	  break;
	}
  }

  fclose (fp);

  //snum ��0�łȂ��ꍇ�́A�I�t�Z�b�g�����̂��Aalti���Ď擾�B
  //���ʂȃA���S���Y�������ǁAsnum��0�̏ꍇ�͊�Ȃ̂ŁA���Ȃ��B

  if (snum != 0) {
	k = 0;
	while (k < snum) {
	  d0.alti[k++] = -9999;
	}

	fp = fopen (argv[1], "r");
	while (1) {
	  fgets (buf, 256, fp);
	  if (strstr (buf, "<gml:tupleList>") != NULL) {
	fgets (buf, 256, fp);
	while (strstr (buf, ",") != NULL) {
	  d0.alti[k++] = (float) atof (cut (buf, ",", "\n", cbuf));
	  fgets (buf, 256, fp);
	}
	break;
	  }
	}
  }
  while (k <= dnum) {
	d0.alti[k++] = -9999;
  }

  //�f�[�^�Ȃ���0�ɂ���B
  if(nodata == 0){
   for(k=0;k<dnum;k++){
	if(d0.alti[k] == -9999) d0.alti[k] = 0;
   }
  }

  char drive[10];
  char dir[256];
  char fname[256];
  char ext[256];
  char outpath[1024] = "";
  _splitpath (argv[1], drive, dir, fname, ext);
  strcat (outpath, drive);
  strcat (outpath, dir);
  strcat (outpath, d0.mesh);
  strcat (outpath, ".tif");
  makeGeotiff (&d0, outpath,nodata);
  free (d0.alti);
  return 0;
}
