/***********************************************************************/
/* FILE : KENDALA.HPP                                                  */
/* SUBPROGRAM UNTUK MENGHITUNG KENDALA                                 */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/
float Kendala_Harga(int var_b_nya[],int var_k_nya[])
{
   ////////////////////////////////////
   // Inisialisasi Kendala Dan Harga //
   ////////////////////////////////////
   float kendalanya=0.;
   float harganya=0.;

	//////////////////////////////////
   // Pembacaan Data Umum Struktur //
   //////////////////////////////////
   baca_data();

   //////////////////////////////////////////////
   // Membaca Beban Yang Bekerja Pada Struktur //
   //////////////////////////////////////////////
   baca_beban();

   ///////////////////////////////////
   // Menghitung Gaya-Gaya Struktur //
   ///////////////////////////////////
   inersia();
	struktur();

   /////////////////////////////////////////
   // Menghitung Kendala Dan Harga Balok //
   /////////////////////////////////////////
   for(int iKH=0;iKH<jum_balok;iKH++)
   {
      //////////////////////////////////
   	// Memerikasa Kemiringan Batang //
      //////////////////////////////////
      periksa_batang(no_balok[iKH]);

      ////////////////////////////
      // Pengisian Elemen Balok //
      ////////////////////////////
      B=isi(var_b_nya[0+(12*iKH)],sisi_d_B);
      H=isi(var_b_nya[1+(12*iKH)],sisi_d_H);

      DIA1lap=isi(var_b_nya[2+(12*iKH)],DIA_d);
      NL1lap=isi(var_b_nya[3+(12*iKH)],NL_d);
      DIA2lap=isi(var_b_nya[4+(12*iKH)],DIA_d);
      NL2lap=isi(var_b_nya[5+(12*iKH)],NL_d);

      DIA1tum=isi(var_b_nya[6+(12*iKH)],DIA_d);
      NL1tum=isi(var_b_nya[7+(12*iKH)],NL_d);
      DIA2tum=isi(var_b_nya[8+(12*iKH)],DIA_d);
      NL2tum=isi(var_b_nya[9+(12*iKH)],NL_d);

      DIAS=isi(var_b_nya[10+(12*iKH)],DIAS_d);
      Jarak_S=isi(var_b_nya[11+(12*iKH)],JS_d);
      L=EL[no_balok[iKH]];

      ////////////////////////////////////////
      // Menentukan Gaya Geser Yang Terjadi //
      ////////////////////////////////////////
      if(fabs(GESER_KI[no_balok[iKH]])>GESER_KA[no_balok[iKH]])
      {
     		VU = fabs(GESER_KI[no_balok[iKH]]/teta);
      }
      else
      {
         VU = fabs(GESER_KA[no_balok[iKH]]/teta);
      }

      /////////////////////
      // Daerah Lapangan //
      /////////////////////
      DIA1=DIA1lap;
      NL1=NL1lap;
      DIA2=DIA2lap;
      NL2=NL2lap;
      MU=fabs(MLAP[no_balok[iKH]]/teta);

      ///////////////////////////////////////
      // Membangkitkan Balok Pada Lapangan //
      ///////////////////////////////////////
      balok lapangan;
      kendalanya+=kendala;
      harganya+=harga;

      lendutan(no_balok[iKH]);
      kendalanya+=kendala_lendutan;

      ////////////////////
      // Daerah Tumpuan //
      ////////////////////
      DIA1=DIA1tum;
      NL1=NL1tum;
      DIA2=DIA2tum;
      NL2=NL2tum;

      if(fabs(MTUM_KI[no_balok[iKH]])>fabs(MTUM_KA[no_balok[iKH]]))
	   {
   	  	MU=fabs(MTUM_KI[no_balok[iKH]]/teta);
     	}
      else
      {
      	MU=fabs(MTUM_KA[no_balok[iKH]]/teta);
      }

      //////////////////////////////////////
      // Membangkitkan Balok Pada Tumpuan //
      //////////////////////////////////////
      balok tumpuan;
      kendalanya+=kendala;
      harganya+=harga;
   }

   /////////////////////////////////////////////////
   // Menghitung Kendala Dan Harga Kolom Biaksial //
   /////////////////////////////////////////////////
   for(int iKH=0;iKH<jum_kolom;iKH++)
   {
      /////////////////////////////////
   	// Memeriksa Kemiringan Batang //
      /////////////////////////////////
      periksa_batang(no_kolom[iKH]);

      ///////////////////////////
      // Pengisian Data Elemen //
      ///////////////////////////
      sisi   =isi(var_k_nya[0+(5*iKH)],sisi_d_K); // sisi kolom (mm)
      DIA    =isi(var_k_nya[1+(5*iKH)],DIA_d);    // Diameter tulangan
                                                  // utama (mm)

      N_DIA  =isi(var_k_nya[2+(5*iKH)],NL_d);   // Jumlah tulangan
      DIAS   =isi(var_k_nya[3+(5*iKH)],DIAS_d); // Diameter sengkang (mm)
      Jarak_S=isi(var_k_nya[4+(5*iKH)],JS_d);   // Jarak antara
                                                // sengkang (mm)

      ////////////////////////////////////
      // Gaya Aksial Yang Harus Ditahan //
      ////////////////////////////////////
      PU  =fabs((PK[no_kolom[iKH]])/(teta)); // (N)

      //////////////////////////////
      // Momen Yang Harus Ditahan //
      //////////////////////////////
      MUX =(MKX[no_kolom[iKH]])/(teta);	// (Nm)
      MUY =(MKY[no_kolom[iKH]])/(teta);	// (Nm)

      ///////////////////////////////////
      // Gaya Geser Yang Harus Ditahan //
      ///////////////////////////////////
      VU  =fabs((GK[no_kolom[iKH]])/(teta)); // (N)

      L=EL[no_kolom[iKH]]; // Panjang kolom (m)

      /////////////////////////
      // Membangkitkan Kolom //
      /////////////////////////
      kolom bangkit;


      for(int cari_S=(nvk[4+(5*iKH)]-1);
        		  cari_S>0;cari_S--)
      {
        	if(isi(cari_S,JS_d)<=Sref)
        	{
        		var_k_nya[4+(5*iKH)]=cari_S;
           	Jarak_S=isi(cari_S,JS_d);
           	break;
        	}
  		}
         kolom raise;   

      ///////////////////////////////////////////////////
 		// Bila Ada Kendala Kelangsingan Ubah Nilai Sisi //
      ///////////////////////////////////////////////////
      if(kendala_kelangsingan>0)
      {
         L=EL[no_kolom[iKH]]; // Panjang kolom (m)
      	float sisi_baru=((0.75*L*1000.)/(22.*sqrt((1./12.))));
        	for(int cari_sisi=var_k_nya[0+(5*iKH)];
           		  cari_sisi<nvk[0+(5*iKH)];cari_sisi++)
         {
            float sisi_cari=isi(cari_sisi,sisi_d_K);
           	if(sisi_cari>sisi_baru)
            {
              	var_k_nya[0+(5*iKH)]=cari_sisi;
               sisi=isi(cari_sisi,sisi_d_K);
               break;
            }
  			}
	      /////////////////////////
   	   // Hitung Kendala Baru //
       	/////////////////////////
         kolom bangkit;
      }

      kendalanya+=kendala;
      harganya+=harga;
   }
	return(finalti/(harganya+(finalti*kendalanya)));
}

//---------------------------------------------------------------------//
