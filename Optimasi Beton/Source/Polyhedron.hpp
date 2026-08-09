/***********************************************************************/
/* FILE : POLYHEDRON.HPP                                               */
/* OPTIMASI STRUKTUR DENGAN METODA FLEXIBLE POLYHEDRON                 */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/

/***************************************/
/* SUBPROGRAM UNTUK MELAKUKAN OPTIMASI */
/***************************************/
void optimasi()
{
   ///////////////////////////////////////////////////////////////
   // Pembangkitan Kelas Ofstream Untuk Penanganan Masalah File //
   ///////////////////////////////////////////////////////////////
	ofstream opti(fhistory);

   //////////////////////////////////
 	// Membaca Data Input Dari File //
  	//////////////////////////////////
	baca_data();

   ////////////////////////////////////////////////////////////////
   // Menghitung Jumlah Balok Kolom Dan Identifikasi Balok Kolom //
   ////////////////////////////////////////////////////////////////
   jum_balok=0;
   jum_kolom=0;
   for(int iop=1;iop<=M;iop++)
  	{
      /////////////////////////////////
		// Memeriksa Kemiringan Batang //
      /////////////////////////////////
  		periksa_batang(iop);

      //////////////////////////////////
      // Batang Non Vertikal -> Balok //
      //////////////////////////////////
     	if(CXZ>0.001)
      {
         no_balok[jum_balok]=iop;
      	jum_balok ++;
      }

      //////////////////////////////
      // Batang Vertikal -> Kolom //
      //////////////////////////////
      else
      {
         no_kolom[jum_kolom]=iop;
      	jum_kolom++;
      }
   }

   ///////////////////////////////////////
   // Menghitung Jumlah Variabel Desain //
   ///////////////////////////////////////
   JVD=(12*jum_balok)+(5*jum_kolom);

   ///////////////////////////////////////
   // Jumlah Struktur Yang Dibangkitkan //
   ///////////////////////////////////////
   JSTD=(JVD*fak_kali)+fak_plus;

   ///////////////////////
   // Mengacak Variabel //
   ///////////////////////
   acak_variabel();

   /////////////////////////////////
   // Mengisi Batas Atas Variabel //
   /////////////////////////////////
   load_batas_atas();

	///////////////////////////////////////////
   // Penormalan Batas Variabel Balok Kolom //
   ///////////////////////////////////////////
   normalisasi_int(nvm,nvb,nvk);

   clrscr();
   about();
   cout << "Generasi ke - 1\n";
   cout << "\nJumlah Struktur desain = \n";
   cout << "\nWaktu yang telah berjalan : 0 detik\n";
   cout << "\nEstimasi waktu yang diperlukan : ?\n";

   ///////////////////////////////////////////////////////////
   // Pembangkitan Generasi Pertama                         //
   // Struktur Dibangkitkan Sebanyak Jumlah Struktur Desain //
	///////////////////////////////////////////////////////////
	for(int iop=0;iop<JSTD;iop++)
   {
     	////////////////////////////////////
	   // Inisialisasi Variabel Struktur //
     	////////////////////////////////////
      no_struktur=iop;
      kendalastr[iop]=0.;
   	hargastr[iop]=0.;;
     	fitstr[iop]=0.;

      /////////////////////////////////////////////////////////
     	// Pencatatan Jumlah Struktur Desain Yang Dibangkitkan //
	   /////////////////////////////////////////////////////////
      gotoxy(26,9);
     	cout << (iop+1) << "  ";

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

     	////////////////////////////////////////
	   // Menghitung Kendala Dan Harga Balok //
      ////////////////////////////////////////
	  	for(int jop=0;jop<jum_balok;jop++)
   	{
         /////////////////////////////////
      	// Memeriksa Kemiringan Batang //
         /////////////////////////////////
	      periksa_batang(no_balok[jop]);

         ///////////////////////////////////
         // Mengisi Properti Elemen Balok //
         ///////////////////////////////////
         isi_elemen_balok(jop);

         ///////////////////////////////////////
	      // Membangkitkan Balok Pada Lapangan //
   	   ///////////////////////////////////////
         elemen_lapangan(jop);
	      balok lapangan;

         //////////////////////////////
         // Mengganti jarak sengkang //
         //////////////////////////////
        	for(int cari_S=(nvb[11+(12*jop)]-1);
         	  	  cari_S>0;cari_S--)
	      {
   	   	if(isi(cari_S,JS_d)<=Sref)
      	   {
         		var_b[no_struktur][11+(12*jop)]=cari_S;
             	Jarak_S=isi(cari_S,JS_d);
            	break;
	         }
  		  	}
         balok raise_lap;

         kendalastr[iop]+=kendala;
      	hargastr[iop]+=harga;

         /////////////////////////////////////////////
         // Menghitung Lendutan Pada Tengah Bentang //
         /////////////////////////////////////////////
         lendutan(no_balok[jop]);
         kendalastr[iop]+=kendala_lendutan;

         //////////////////////////////////////
      	// Membangkitkan Balok Pada Tumpuan //
         //////////////////////////////////////
         elemen_tumpuan(jop);
	      balok tumpuan;    
         kendalastr[iop]+=kendala;
	      hargastr[iop]+=harga;
	  	}

   	////////////////////////////////////////
      // Menghitung Kendala Dan Harga Kolom //
	   ////////////////////////////////////////
   	for(int jop=0;jop<jum_kolom;jop++)
		{
         /////////////////////////////////
	   	// Memeriksa Kemiringan Batang //
         /////////////////////////////////
   	   periksa_batang(no_kolom[jop]);

         ///////////////////////////////////
         // Mengisi Properti Elemen Kolom //
         ///////////////////////////////////
         isi_elemen_kolom(jop);

   	   /////////////////////////
      	// Membangkitkan Kolom //
         /////////////////////////
   	   kolom bangkit;

         //////////////////////////////
         // Mengganti jarak sengkang //
         //////////////////////////////
        	for(int cari_S=(nvk[4+(5*jop)]-1);
           		  cari_S>0;cari_S--)
       	{
           	if(isi(cari_S,JS_d)<=Sref)
           	{
           		var_k[no_struktur][4+(5*jop)]=cari_S;
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
            L=EL[no_kolom[jop]];
            float sisi_baru=((0.75*L*1000.)/(22.*sqrt((1./12.))));
            for(int cari_sisi=var_k[no_struktur][0+(5*jop)];
             		  cari_sisi<nvk[0+(5*jop)];cari_sisi++)
            {
               float sisi_cari=isi(cari_sisi,sisi_d_K);
             	if(sisi_cari>sisi_baru)
               {
                 	var_k[no_struktur][0+(5*jop)]=cari_sisi;
                  sisi=isi(cari_sisi,sisi_d_K);
                  break;
               }
  				}
            /////////////////////////
            // Hitung Kendala Baru //
            /////////////////////////
            kolom bangkit;
         }

         kendalastr[iop]+=kendala;
	      hargastr[iop]+=harga;
   	}

      /////////////////////////////////
      // Menghitung Fitness Struktur //
      /////////////////////////////////
	   fitstr[iop]=(finalti/(hargastr[iop]+(finalti*kendalastr[iop])));
  	}

	////////////////////////////////
	// Lakukan Pengurutan Fitness //
   ////////////////////////////////
	sort(fitstr,kendalastr,hargastr,var_b,var_k,JSTD);

   //////////////////////////////
   // Pengambilan waktu sistem //
   //////////////////////////////
   time_t t_awal, t_sekarang, t_akhir;
   time(&t_awal);

   ////////////////////////////////////////////
   // Proses Pencarian Hasil Optimal Dimulai //
   ////////////////////////////////////////////
   int generasi=1;
   jum_susut=0;
   do
   {
      ////////////////////////////
      // Menulis Nomor Generasi //
      ////////////////////////////
   	gotoxy(15,7);
   	cout << (generasi+1) << "  ";

   	///////////////////////////////
	   // Menbangkitkan Kelas Timer //
      ///////////////////////////////
	   time(&t_sekarang);

      /////////////////////////////////////////////
	   // Menulis Lama Waktu Yang Sedang Berjalan //
      /////////////////////////////////////////////
	   gotoxy(29,11);
	   cout << difftime(t_sekarang,t_awal) << " detik               ";

      ////////////////////////////
	   // Menulis Estimasi Waktu //
      ////////////////////////////
	   gotoxy(34,13);
	   cout << int((difftime(t_sekarang,t_awal)/(generasi+1))
      		  	*(j_iterasi_mak-generasi)) << " detik                ";

      /////////////////////////////////
  	 	// Menentukan Arah Penelusuran //
		/////////////////////////////////
	   penelusuran();

   	///////////////////////////////////////////////////////
   	// Mencari Titik Baru Untuk Menggantikan Titik Worst //
   	///////////////////////////////////////////////////////
   	cari_baru();

   	/////////////////////////////////////////
   	// Membandingkan Fitness Struktur Baru //
   	/////////////////////////////////////////
      if((fitcb_best)>(fitstr[0]))
	   {
         if(jum_susut>1)
         {
            opti << "     Penyusutan berturut berakhir setelah "
                 << jum_susut << " kali\n";
         	jum_susut=0;
         }

         //////////////////////////////////////////////////
   		// Ganti Variabel Terjelek Dengan Variabel Baru //
         //////////////////////////////////////////////////
         ganti_baru();

	   }
   	else
	   {
         ////////////////////////
   		// Lakukan Penyusutan //
         ////////////////////////
      	penyusutan();
         jum_susut++;
         if(jum_susut==1)
         {
         	patok_fit=fitstr[JSTD-1];
            opti << "Penyusutan berturut pertama pada generasi : " << generasi
                 << " Fitness terbaik : " << fitstr[JSTD-1] << endl;
         }
	   }

		////////////////////////////////////
	   // Inisialisasi Variabel Struktur //
     	////////////////////////////////////
      no_struktur=0;
      kendalastr[0]=0.;
   	hargastr[0]=0.;;
     	fitstr[0]=0.;

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

     	////////////////////////////////////////
	   // Menghitung Kendala Dan Harga Balok //
      ////////////////////////////////////////
	  	for(int iop=0;iop<jum_balok;iop++)
   	{
	      periksa_batang(no_balok[iop]);
         isi_elemen_balok(iop);
         elemen_lapangan(iop);
	      balok lapangan;
         kendalastr[0]+=kendala;
      	hargastr[0]+=harga;
         lendutan(no_balok[iop]);
         kendalastr[0]+=kendala_lendutan;
         elemen_tumpuan(iop);
	      balok tumpuan;
         kendalastr[0]+=kendala;
	      hargastr[0]+=harga;
	  	}

   	////////////////////////////////////////
      // Menghitung Kendala Dan Harga Kolom //
	   ////////////////////////////////////////
   	for(int iop=0;iop<jum_kolom;iop++)
		{
	   	periksa_batang(no_kolom[iop]);
         isi_elemen_kolom(iop);
   	   kolom bangkit;
         kendalastr[0]+=kendala;
	      hargastr[0]+=harga;
   	}
	   fitstr[0]=finalti/(hargastr[0]+(finalti*kendalastr[0]));

      ////////////////////////////////
		// Lakukan Pengurutan Fitness //
   	////////////////////////////////
		sort(fitstr,kendalastr,hargastr,var_b,var_k,JSTD);

      /////////////////////////
      // Pencetakan Ke Layar //
      /////////////////////////
      gotoxy(1,17);
      cout << "Fitness terbaik : " << fitstr[JSTD-1]
           << "              \n\n";
      cout << "Harga : " << hargastr[JSTD-1]
           << "              \n\n";
      cout << "Kendala : " << kendalastr[JSTD-1]
           << "              \n\n";

      ///////////////////////////
      // Memeriksa Konvergensi //
      ///////////////////////////
      if(fitstr[JSTD-1]==fitstr[(JSTD-JVD)-1])
      {
         opti << "Konvergen pada generasi : " << generasi << endl;
         break;
      }

      //////////////////////////////////////////////////////////
      // Accelerator Konvergensi                              //
      // Dijalankan Apabila Telah Terjadi 0.5*JSTD Penyusutan //
      //////////////////////////////////////////////////////////
      if(jum_susut==JSTD)
      {
         ////////////////////////////////////////////////////////
         // Hentikan Iterasi Apabila Setelah JVD Kali Menyusut //
         // Dan Fitness Terbaik Tetap                          //
         ////////////////////////////////////////////////////////
         if(patok_fit==fitstr[JSTD-1])
         {
            opti << "Konvergen Pada Generasi : " << generasi
            << " karena jumlah penyusutan melebihi batas" << endl;
         	break;
         }

         ////////////////////////////////////
         // Jumlah Penyusutan DiReset Lagi //
         ////////////////////////////////////
         jum_susut=0;
      }

      ///////////////
      // Increator //
      ///////////////
      generasi++;
   }while(generasi<j_iterasi_mak);

   ///////////////////////////
   // Mengambil Waktu Akhir //
   ///////////////////////////
   time(& t_akhir);

   ////////////////////
   // Cetak Ke Layar //
   ////////////////////
   gotoxy(1,17);
   cout << "Waktu optimasi : " << difftime(t_akhir,t_awal)
        << " detik            \n\a";
   cout << "\nFitness Terbaik : " << fitstr[JSTD-1]
        << "          " << endl;
   cout << "Harga : " << hargastr[JSTD-1]
        << "          " << endl;
   cout << "Kendala : " << kendalastr[JSTD-1]
        << "          " << endl;
   gotoxy(55,24);
   cout << "Tekan <CR> untuk keluar";
   getch();

   ///////////////////
   // Cetak Ke File //
   ///////////////////
   if (generasi==j_iterasi_mak)
   {
   	opti << "Selesai pada generasi " << (generasi)
           << " karena jumlah iterasi mencapai jumlah iterasi maksimum\n";
   }
  	opti << "Fitness = " << fitstr[JSTD-1] << endl;
   opti << "Harga = " << hargastr[JSTD-1] << endl;
	opti << "Kendala = " << kendalastr[JSTD-1] << endl;
   opti << "Waktu optimasi : " << difftime(t_akhir,t_awal)
        << " detik       ";
   opti.close();
   cetak_akhir();
}

//---------------------------------------------------------------------//
