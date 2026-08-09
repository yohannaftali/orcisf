/***********************************************************************/
/* FILE : PENORMALAN.HPP                                               */
/* PENANGANAN MASALAH VARIABEL BALOK DAN KOLOM                         */
/* DENGAN VARIABEL GABUNGAN BALOK KOLOM                                */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/

/*****************************************************/
/* SUBPROGRAM MENGEMBALIKAN VARIABEL BALOK DAN KOLOM */
/*****************************************************/
void unnormalisasi(int var_nor[mak],int var_bv[mak],int var_kv[mak])
{
   int novar=0;
	for(int isum=0;isum<jum_balok;isum++)
   {
     	for(int jsum=0;jsum<12;jsum++)
     	{
      	var_bv[jsum+(12*isum)]=var_nor[novar];
         novar++;
      }
   }
   for(int isum=0;isum<jum_kolom;isum++)
   {
     	for(int jsum=0;jsum<5;jsum++)
   	{
    		var_kv[jsum+(5*isum)]=var_nor[novar];
         novar++;
     	}
   }
}

/*******************************************************************/
/* SUBPROGRAM MENORMALISASIKAN VARIABEL BALOK DAN KOLOM TIPE FLOAT */
/*******************************************************************/
void normalisasi_float(float var_nor[],float var_bv[],float var_kv[])
{
   int novar=0;
	for(int isum=0;isum<jum_balok;isum++)
   {
     	for(int jsum=0;jsum<12;jsum++)
     	{
      	var_nor[novar]=var_bv[jsum+(12*isum)];
         novar++;
      }
   }
   for(int isum=0;isum<jum_kolom;isum++)
   {
     	for(int jsum=0;jsum<5;jsum++)
   	{
    		var_nor[novar]=var_kv[jsum+(5*isum)];
         novar++;
     	}
   }
}

/*********************************************************************/
/* SUBPROGRAM MENORMALISASIKAN VARIABEL BALOK DAN KOLOM TIPE INTEGER */
/*********************************************************************/
void normalisasi_int(int var_nor[],int var_bv[],int var_kv[])
{
   int novar=0;
	for(int isum=0;isum<jum_balok;isum++)
   {
     	for(int jsum=0;jsum<12;jsum++)
     	{
      	var_nor[novar]=var_bv[jsum+(12*isum)];
         novar++;
      }
   }
   for(int isum=0;isum<jum_kolom;isum++)
   {
     	for(int jsum=0;jsum<5;jsum++)
   	{
    		var_nor[novar]=var_kv[jsum+(5*isum)];
         novar++;
     	}
   }
}

//---------------------------------------------------------------------//
