/*************************************************************************/
/* FILE : DISKRITISASI.HPP                										 */
/* SUBPROGRAM UNTUK MELAKUKAN KONVERSI MENJADI BILANGAN DISKRIT          */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                                */
/*************************************************************************/
int konversi(float bil_asli)
{
   if((fabs(bil_asli)-(abs(bil_asli)))<=0.5)
   {
   	bil_asli=floor(bil_asli);
   }
   else
   {
   	bil_asli=ceil(bil_asli);
   }
   return bil_asli;
}

//-----------------------------------------------------------------------//