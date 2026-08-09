/***********************************************************************/
/* FILE : PENGACAKAN.HPP                                               */
/* PUSTAKA URUSAN VARIABEL DESAIN ACAK                                 */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/

/******************************************/
/* SUBPROGRAM PENGENDALI LOOP RANDOMISASI */
/******************************************/
void acak_variabel()
{
	///////////////////////////////////////////////////
   // Pengacakan Seed Untuk Random Number Generator //
   ///////////////////////////////////////////////////
   randomize();

   load_batas_atas();

   for(int iav=0;iav<JSTD;iav++)
   {
   	no_struktur=iav;

      if(iav==1)
      {
        	cari_struktur_awal();
       	continue;
     	}
      
   	/////////////////////////////////
      // Random Struktur Selanjutnya //
      /////////////////////////////////
   	randomisasi();
   }
}

/******************************************/
/* SUBPROGRAM UNTUK MELAKUKAN RANDOMISASI */
/******************************************/
void randomisasi()
{
   ///////////////////////////////////////
   // Randomisasi Variabel Desain Balok //
   ///////////////////////////////////////
   for(int iran=0;iran<jum_balok;iran++)
   {
      for(int nv_b=0;nv_b<12;nv_b++)
      {
         var_b[no_struktur][nv_b+(12*iran)]=random(nvb[nv_b]);
      }
   }

   ///////////////////////////////////////
   // Randomisasi Variabel Desain Kolom //
   ///////////////////////////////////////
   for(int iran=0;iran<jum_kolom;iran++)
   {
      for(int nv_k=0;nv_k<5;nv_k++)
      {
        	var_k[no_struktur][nv_k+(5*iran)]=random(nvk[nv_k]);
      }
   }
}

/**********************************************************/
/* SUBPROGRAM UNTUK MENENTUKAN BATAS ATAS VARIABEL DESAIN */
/**********************************************************/
void load_batas_atas()
{
	for(int isinv=0;isinv<jum_balok;isinv++)
   {
   	nvb[0+(12*isinv)]=nsisi_B;// B
   	nvb[1+(12*isinv)]=nsisi_H;// H
   	nvb[2+(12*isinv)]=nDIA;   // Diameter tulangan tarik lapangan
   	nvb[3+(12*isinv)]=nNL;    // jumlah tulangan tarik lapangan
   	nvb[4+(12*isinv)]=nDIA;   // Diameter tulangan desak lapangan
		nvb[5+(12*isinv)]=nNL;    // jumlah tulangan desak lapangan
   	nvb[6+(12*isinv)]=nDIA;   // Diameter tulangan tarik tumpuan
   	nvb[7+(12*isinv)]=nNL;    // jumlah tulangan tarik tumpuan
   	nvb[8+(12*isinv)]=nDIA;   // Diameter tulangan desak tumpuan
   	nvb[9+(12*isinv)]=nNL;    // jumlah tulangan desak tumpuan
		nvb[10+(12*isinv)]=nDIAS; // Diameter tulangan sengkang
   	nvb[11+(12*isinv)]=nJS;   // jarak antar tulangan sengkang
   }
   for(int isinv=0;isinv<jum_kolom;isinv++)
   {
   	nvk[0+(5*isinv)]=nsisi_K;// Sisi B=H
   	nvk[1+(5*isinv)]=nDIA;   // Diameter tulangan bawah lapangan
   	nvk[2+(5*isinv)]=nNL;    // jumlah tulangan pada satu sisi
   	nvk[3+(5*isinv)]=nDIAS;  // Diameter tulangan sengkang
   	nvk[4+(5*isinv)]=nJS;    // jarak antar tulangan sengkang
   }
}

void cari_struktur_awal()
{
   for(int iran=0;iran<jum_balok;iran++)
   {
   	for(int nv_b=0;nv_b<12;nv_b++)
   	{
        	var_b[no_struktur][nv_b+(12*iran)]=(nvb[nv_b]-1);
        	if(nv_b==5||nv_b==9||nv_b==11)
        	{
        		var_b[no_struktur][nv_b+(12*iran)]=0;
     	 	}
      }
   }

   for(int iran=0;iran<jum_kolom;iran++)
   {
    	for(int nv_k=0;nv_k<5;nv_k++)
      {
      	var_k[no_struktur][nv_k+(5*iran)]=(nvk[nv_k]-1);
      }
   }
}  

//---------------------------------------------------------------------//
