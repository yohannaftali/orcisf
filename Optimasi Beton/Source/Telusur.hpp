/**********************************************************************/
/* FILE : TELUSUR.HPP                                                 */
/* SUBPROGRAM UNTUK MELAKUKAN PENELUSURAN MENUJU TITIK BARU           */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                             */
/**********************************************************************/
void penelusuran()
{
	///////////////////////////////
   // Mencari Letak Titik Midle //
   ///////////////////////////////

   ///////////////////////
   // Inisialisasi Awal //
   ///////////////////////
   float sum_bestgood_b[mak];
   float sum_bestgood_k[mak];
   for(int isum=0;isum<jum_balok;isum++)
   {
    	for(int jsum=0;jsum<12;jsum++)
     	{
     		sum_bestgood_b[jsum+(12*isum)]=0.;
     	}
   }
   for(int isum=0;isum<jum_kolom;isum++)
   {
   	for(int jsum=0;jsum<5;jsum++)
   	{
   		sum_bestgood_k[jsum+(5*isum)]=0.;
   	}
   }

   //////////////////////////////////////////////////////
   // Jumlah Kordinat = 12*jum_balok_p + 5*jum_kolom_p //
   // Menjumlahkan Variabel Best Dan Good Pada Balok   //
   //////////////////////////////////////////////////////
   for(int nbgw=1;nbgw<JSTD;nbgw++)
   {
      ////////////////////////////
	   // 12*jum_balok_p Titik M //
      ////////////////////////////
     	for(int isum=0;isum<jum_balok;isum++)
    	{
     		for(int jsum=0;jsum<12;jsum++)
     		{
     			sum_bestgood_b[jsum+(12*isum)]+=var_b[nbgw][jsum+(12*isum)];
     		}
     	}

      ///////////////////////////
      // 5*jum_kolom_p Titik M //
      ///////////////////////////
     	for(int isum=0;isum<jum_kolom;isum++)
     	{
     		for(int jsum=0;jsum<5;jsum++)
    		{
     			sum_bestgood_k[jsum+(5*isum)]+=var_k[nbgw][jsum+(5*isum)];
     		}
     	}
   }

   ////////////////////////////////////////////////////
   // Dari Sini Kita Mempunyai jum_kolom_p Koordinat //
   // Dan jum_balok_p Koordinat                      //
   // Menentukan Koordinat Titik M                   //
   ////////////////////////////////////////////////////
   for(int isum=0;isum<jum_balok;isum++)
   {
     	for(int jsum=0;jsum<12;jsum++)
     	{
      	XM_b[jsum+(12*isum)]=sum_bestgood_b[jsum+(12*isum)]/(JSTD-1.);
     	}
   }
   for(int isum=0;isum<jum_kolom;isum++)
   {
     	for(int jsum=0;jsum<5;jsum++)
   	{
    		XM_k[jsum+(5*isum)]=sum_bestgood_k[jsum+(5*isum)]/(JSTD-1.);
     	}
   }


   /////////////////////////////////////////
   // Mencari Arah Penelusuran Pada Balok //
   /////////////////////////////////////////
   for(int isum=0;isum<jum_balok;isum++)
   {
     	for(int jsum=0;jsum<12;jsum++)
     	{
      	XS_b[jsum+(12*isum)]=(XM_b[jsum+(12*isum)]
           						  -var_b[0][jsum+(12*isum)]);
         if(XS_b[jsum+(12*isum)]>0.)
         {
           	tanda_arah_b[jsum+(12*isum)]=1.;
         }
         else
      	{
        		if(XS_b[jsum+(12*isum)]<0)
            {
             	tanda_arah_b[jsum+(12*isum)]=(-1.);
            }
         	if(XS_b[jsum+(12*isum)]==0)
            {
              	tanda_arah_b[jsum+(12*isum)]=(0.);
            }
     		}
     	}
   }

   /////////////////////////////////////////
   // Mencari Arah Penelusuran Pada Kolom //
   /////////////////////////////////////////
   for(int isum=0;isum<jum_kolom;isum++)
   {
     	for(int jsum=0;jsum<5;jsum++)
     	{
     		XS_k[jsum+(5*isum)]=(XM_k[jsum+(5*isum)]
          						 -var_k[0][jsum+(5*isum)]);
         if(XS_k[jsum+(5*isum)]>0.)
         {
           	tanda_arah_k[jsum+(5*isum)]=1.;
         }
         else
      	{
        		if(XS_k[jsum+(5*isum)]<0)
            {
              	tanda_arah_k[jsum+(5*isum)]=(-1.);
            }
         	if(XS_k[jsum+(5*isum)]==0)
           	{
              	tanda_arah_k[jsum+(5*isum)]=(0.);
            }
      	}
      }
   }

   ///////////////////////////////////
   // Penggandaan Variabel Terjelek //
   ///////////////////////////////////

   /////////////////////////////////////////////
   // 12*jum_balok_p Koordinat Titik Terjelek //
   /////////////////////////////////////////////
   for(int isum=0;isum<jum_balok;isum++)
  	{
  		for(int jsum=0;jsum<12;jsum++)
  		{
  			var_b_jelek[jsum+(12*isum)]=var_b[0][jsum+(12*isum)];
  		}
  	}

   ////////////////////////////////////////////
   // 5*jum_kolom_p Koordinat Titik Terjelek //
   ////////////////////////////////////////////
  	for(int isum=0;isum<jum_kolom;isum++)
  	{
  		for(int jsum=0;jsum<5;jsum++)
 		{
  			var_k_jelek[jsum+(5*isum)]=var_k[0][jsum+(5*isum)];
  		}
  	}

   ///////////////////////////
   // Inisialisasi Awal = 0 //
   ///////////////////////////
   for(int isum=0;isum<JVD;isum++)
  	{
      TM[isum]=0.;
      TS[isum]=0.;
  		varnew_asli[isum]=0;
      arah[isum]=0;
  	}
   normalisasi_float(TM,XM_b,XM_k);
   normalisasi_float(TS,XS_b,XS_k);
   normalisasi_int(arah,tanda_arah_b,tanda_arah_k);

   /////////////////////////////
   // Cari nomor Arah Terjauh //
   /////////////////////////////
   no_TS_terjauh=0;
   for(int cari_ts=1;cari_ts<JVD;cari_ts++)
   {
   	if(fabs(TS[cari_ts])>fabs(TS[no_TS_terjauh]))
      {
      	no_TS_terjauh=cari_ts;
      }
   }
}

//---------------------------------------------------------------------//
