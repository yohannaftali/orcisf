/***********************************************************************/
/* FILE : BARU.HPP                                                     */
/* PUSTAKA PENANGANAN TITIK BARU                                       */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/

/***************************************/
/* SUBPROGRAM UNTUK MENCARI TITIK BARU */
/***************************************/
void cari_baru()
{
	///////////////////////
   // Inisialisasi Awal //
   ///////////////////////
   iterasi_var=0;

   normalisasi_int(varnew_asli,var_b_jelek,var_k_jelek);
   fitcb_best=0.;
	do
   {
      gotoxy(1,15);
      cprintf ("Pencarian arah baru ke - %d",(iterasi_var+1));
      cprintf ("         ");

      //////////////////////////////////////
		// Menentukan Titik X Pertama Baru  //
      // Diambil Yang Arahnya Paling Jauh //
      /////////////////////////////////////
      varplus[no_TS_terjauh]=(iterasi_var+1.);

      //////////////////////////////////////////////
      // Variabel Baru Ditambahkan Sesuai Arahnya //
      //////////////////////////////////////////////
      varnew_f[no_TS_terjauh]=varnew_asli[no_TS_terjauh]
      							  +varplus[no_TS_terjauh]
                             *arah[no_TS_terjauh];

      ////////////////////////////
      // Menentukan X Baru Lain //
      ////////////////////////////
      for (int icb=0;icb<JVD;icb++)
      {
         ///////////////////////////////////////////////////////
         // Skip Bila icb Adalah Nomor arah Pencarian Terjauh //
         ///////////////////////////////////////////////////////
         if(icb==no_TS_terjauh)
         {
         	continue;
         }

         if(TS[no_TS_terjauh]!=0)
         {
         	//////////////////////////////
	         // Menghitung Penambahannya //
   	      //////////////////////////////
      	  	varplus[icb]=fabs((TS[icb]/TS[no_TS_terjauh])
         	            *varplus[no_TS_terjauh]);
	         varnew_f[icb]=varnew_asli[icb]
   	                   +varplus[icb]*arah[icb];
         }
         else
         {
				varnew_f[icb]=varnew_asli[icb]
	  	                   +((iterasi_var+1.)*arah[icb]);
         }
      }

      for(int icb=0;icb<JVD;icb++)
      {
        	varnew[icb]=konversi(varnew_f[icb]);
      }

      /////////////////////////////////////////////////////////////////
      // Memeriksa Apakah Titik Baru Tersebut Identik Dengan Titik M //
      /////////////////////////////////////////////////////////////////
      for(int ilp=0;ilp<JVD;ilp++)
      {
      	if(varnew[ilp]==TM[ilp])
         {
         	if(ilp==(JVD-1))
            {
            	lompat=1;
            }
         }
         else
         {
         	lompat=0;
            break;
         }
      }

      ////////////////////////////////////////////////////////////
      // Lompat Dari Loop Apabila Indikator Pelompat bernilai 1 //
      ////////////////////////////////////////////////////////////
      if(lompat==1)
      {
         lompat=0;
         iterasi_var++;
      	continue;
      }   

      periksa_batas();

      /////////////////////////////////////////////////////
      // Perubahan Variabel Umum Ke Variabel Balok Kolom //
      /////////////////////////////////////////////////////
      unnormalisasi(varnew,var_b_cb,var_k_cb);

      ////////////////////////
      // Menghitung Kendala //
      ////////////////////////
     	fitcb=Kendala_Harga(var_b_cb,var_k_cb);

      if(fitcb>fitcb_best)
      {
      	fitcb_best=fitcb;
			for(int icb=0;icb<jum_balok;icb++)
		   {
     			for(int jcb=0;jcb<12;jcb++)
     			{
      			var_b_cb_best[jcb+(12*icb)]=var_b_cb[jcb+(12*icb)];

      		}
         }
   		for(int icb=0;icb<jum_kolom;icb++)
   		{
     			for(int jcb=0;jcb<5;jcb++)
   			{
    				var_k_cb_best[jcb+(5*icb)]=var_k_cb[jcb+(5*icb)];
     			}
   		}
      }
     	iterasi_var++;
   }while(iterasi_var<(fabs(TS[no_TS_terjauh])*3));
}

/*****************************************/
/* SUBPROGRAM UNTUK MEMERIKSA BATAS ATAS */
/* DAN BAWAH VARIABEL DESAIN BARU        */
/* DAN BILA TERJADI PELANGGARAN MAKA     */
/* VARIABEL ITU AKAN DICERMINKAN         */
/*****************************************/
void periksa_batas()
{
   for(int ipb=0;ipb<JVD;ipb++)
   {
     	if((varnew[ipb])>(nvm[ipb]-1))
		{
          varnew[ipb]=(nvm[ipb]-1);
      } 
    	if(varnew[ipb]<0)
   	{
         varnew[ipb]=0;
     	}
   }
}

/*********************************************************************/
/* SUBPROGRAM UNTUK MENGGANTI VARIABEL TERJELEK DENGAN VARIABEL BARU */
/*********************************************************************/
void ganti_baru()
{
   for(int igbar=0;igbar<jum_balok;igbar++)
   {
     	for(int jgbar=0;jgbar<12;jgbar++)
     	{
         var_b[0][jgbar+(12*igbar)]=var_b_cb_best[jgbar+(12*igbar)];
     	}
   }
   for(int igbar=0;igbar<jum_kolom;igbar++)
   {
     	for(int jgbar=0;jgbar<5;jgbar++)
   	{
    		var_k[0][jgbar+(5*igbar)]=var_k_cb_best[jgbar+(5*igbar)];
    	}
   }
}

/************************************************/
/* SUBPROGRAM UNTUK MENYUSUTKAN VARIABEL DESAIN */
/************************************************/
void penyusutan()
{
   for(int nkon=0;nkon<(JSTD-1);nkon++)
   {
   	for(int igbar=0;igbar<jum_balok;igbar++)
   	{
     		for(int jgbar=0;jgbar<12;jgbar++)
     		{
         	var_b[nkon][jgbar+(12*igbar)]=
         	konversi(0.5*(var_b[nkon][jgbar+(12*igbar)]
            +var_b[JSTD-1][jgbar+(12*igbar)]));
     		}
   	}
   	for(int igbar=0;igbar<jum_kolom;igbar++)
   	{
     		for(int jgbar=0;jgbar<5;jgbar++)
   		{
    			var_k[nkon][jgbar+(5*igbar)]=
         	konversi(0.5*(var_k[nkon][jgbar+(5*igbar)]
            +var_k[JSTD-1][jgbar+(5*igbar)]));
    		}
      }
   }
}

//---------------------------------------------------------------------//