/***********************************************************************/
/* FILE : CETAK.HPP					               							  */
/* SUBPROGRAM UNTUK MENCETAK HASIL AKHIR OPTIMASI                      */
/* DITULIS OLEH YOHAN NAFTALI (JULI 1999)                              */
/***********************************************************************/
void cetak_akhir()
{
   ofstream hopt(foptimasi);
   ofstream hstr(fstruktur);
   ofstream hkdl(fkendala);
   ofstream hinf(finformasi);

   //////////////////////////////////
   // Pembacaan Data Umum Struktur //
   //////////////////////////////////
   baca_data();

   //////////////////////////////////////////////
   // Membaca Beban Yang Bekerja Pada Struktur //
   //////////////////////////////////////////////
   baca_beban();

   /////////////////////////////////////////
   // Mencetak Informasi Masukkan Ke File //
   /////////////////////////////////////////
   hinf << "  Struktur Portal Ruang " << ISN << "\n\n";
	hinf << "  Parameter Struktur\n";
   hinf << "  Jumlah batang : " << M << endl;
   hinf << "  DOF : " << N << endl;
   hinf << "  Jumlah joint : " << NJ << endl;
   hinf << "  Jumlah pengekang tumpuan : " << NR << endl;
   hinf << "  Jumlah titik kumpul yang dikekang : " << NRJ << endl;
   hinf << "  Modulus Elastisitas aksial : " << E << " N/m^2\n";
   hinf << "  Modulus Geser : " << G << " N/m^2\n\n";
   hinf << "  Properti Elemen Material\n";
 	hinf << "  Kuat desak beton karakteristik : " << FC << " MPa\n";
   hinf << "  Kuat tarik baja tulangan : "  <<  FY << " MPa\n";
   hinf << "  Kuat tarik tulangan sengkang : " << FYS  << " MPa\n";

   hinf << "\n  Koordinat Titik Kumpul (m)\n";
	hinf << "  Titik     X             Y             Z \n";
	for(int kout=1;kout<=NJ;kout++)
	{
		hinf << setiosflags(ios::left);
		hinf << "  " << setw(10) << kout
      				 << setw(14) << X[kout]
                   << setw(14) << Y[kout]
                   << setw(14) << Z[kout] << endl;
	}

   hinf << "\n  Informasi Batang \n";
	hinf << "  Batang  JJ      JK      IA\n";
	for(int iout=1;iout<=M;iout++)
	{
		hinf << setiosflags(ios::left);
		hinf << "  " << setw(8) << iout
      				 << setw(8) << JJ[iout]
                   << setw(8)<< JK[iout]
		             << setw(3) << IA[iout] << endl;
      if(IA[iout] != 0)
		{
			hinf << setiosflags(ios::left);
			hinf << "  XP = " << setw(12) << XP[iout];
			hinf << "  YP = " << setw(12) << YP[iout] << '\t';
			hinf << "  ZP = " << setw(12) << ZP[iout] << endl;
			hinf << endl;
      }
	}

   hinf << "\n  Pengekang Titik Kumpul \n";
	hinf << "  Titik     JR1   JR2   JR3   JR4   JR5   JR6\n";
	for (int iout=1;iout<=NRJ;iout++)
	{
		hinf << setiosflags(ios::left);
		hinf << "  " << setw(10)<<T_K[iout]
      				 << setw(6) << JRL[6*T_K[iout]-5]
                   << setw(6) << JRL[6*T_K[iout]-4]
                   << setw(6) << JRL[6*T_K[iout]-3]
						 << setw(6) << JRL[6*T_K[iout]-2]
      				 << setw(6) << JRL[6*T_K[iout]-1]
                   << setw(6) << JRL[6*T_K[iout]] << endl;
	}

   hinf << "\n  Gaya di Ujung Batang Terkekang Akibat Beban (Nm)\n"
        << "  Batang  AML1        AML2        AML3        "
   	            << "AML4        AML5        AML6\n"
        << "          AML7        AML8        AML9        "
                  << "AML10       AML11       AML12\n";
   for(int kout=1;kout<=M;kout++)
   {
   	hinf << setiosflags(ios::left);
   	hinf << "  " << setw(8) << kout
      				 << setw(12) << AML[1][kout]
                   << setw(12) << AML[2][kout]
                   << setw(12) << AML[3][kout]
   					 << setw(12) << AML[4][kout]
       	  			 << setw(12) << AML[5][kout]
                   << setw(12) << AML[6][kout] << endl;
      hinf << "          "
      			    << setw(12) << AML[7][kout]
                   << setw(12) << AML[8][kout]
                   << setw(12) << AML[9][kout]
   					 << setw(12) << AML[10][kout]
                   << setw(12) << AML[11][kout]
                   << setw(12) << AML[12][kout] << endl;
   }

   hinf << "\n  Beban Titik (N)\n"
        << "  Titik   Arah 1      Arah 2      Arah 3      "
   	            << "Arah 4      Arah 5      Arah 6\n";
   for(int kout=1;kout<=NJ;kout++)
   {
   	hinf << setiosflags(ios::left);
   	hinf << "  " << setw(8) << kout
                   << setw(12) << AJ[((6*kout)-5)]
						 << setw(12) << AJ[((6*kout)-4)]
  						 << setw(12) << AJ[((6*kout)-3)]
                   << setw(12) << AJ[((6*kout)-2)]
						 << setw(12) << AJ[((6*kout)-1)]
						 << setw(12) << AJ[(6*kout)] << endl;
   }

   ///////////////////////////////////
   // Menghitung Gaya-Gaya Struktur //
   ///////////////////////////////////
   inersia();
   struktur();

   //////////////////////////////////
   // Cetak Hasil Analisa Struktur //
   //////////////////////////////////
   hstr << "Hasil Analisa Struktur Dengan Metoda Kekakuan\n";
   hstr << "Dikembangkan dari Weaver & Gere\n";
   hstr << "Oleh Yohan Naftali 1999\n\n";
   hstr << "Nama File Generik : " << fi << endl;
   hopt << "Nama Struktur : " << ISN << endl;
   hstr << "Jumlah batang : " << M << endl;
   hstr << "Jumlah titik kumpul : " << NJ << endl;
   hstr << "Modulus elastisitas tarik/tekan : "
        << (E*1.E-6) << " MPa" << endl;
   hstr << "Modulus elastisitas geser G : "
        << (G*1.E-6) << " MPa" << endl;
   hstr << "\nPerpindahan Titik Kumpul\n";
	hstr << "Titik  DJ1         DJ2         DJ3  ";
   hstr << "       DJ4         DJ5         DJ6\n";
	for(int cst=1; cst<=NJ; cst++)
	{
		hstr << setiosflags(ios::left);
		hstr << setw(7) << cst;
		hstr << setiosflags( ios::fixed | ios:: showpos);
		hstr << setw(12) << DJ[6*cst-5] << setw(12);
		hstr << DJ[6*cst-4] << setw(12) << DJ[6*cst-3] << setw(12);
		hstr << DJ[6*cst-2] << setw(12) << DJ[6*cst-1] << setw(12);
		hstr << DJ[6*cst] << endl;
      hstr << resetiosflags( ios::fixed | ios:: showpos);
	}

   hstr << "\nGaya Ujung Batang\n";
	hstr << "Batang  AM1         AM2         AM3         ";
   hstr << "AM4         AM5         AM6\n";
	hstr << "        AM7         AM8         AM9         ";
   hstr << "AM10        AM11        AM12\n";

   for(int ih=1;ih<=M;ih++)
	{
      periksa_batang(ih);
		isi_matrik_kekakuan(ih);
      indeks_batang(ih);
     	for(int jh=1;jh<=MD;jh++)
		{
			AMD[jh]=0.;
			for(int kh=1;kh<=MD;kh++)
			{
				AMD[jh]=AMD[jh]+SMRT[jh][kh]*DJ[IM[kh]];
			}
         AM[ih][jh]=AML[jh][ih]+AMD[jh];
		}
      hstr << setiosflags(ios::left);
      hstr << setw(8) << ih;
      hstr << setw(12) << AM[ih][1]
      	  << setw(12) << AM[ih][2]
           << setw(12) << AM[ih][3]
			  << setw(12) << AM[ih][4]
      	  << setw(12) << AM[ih][5]
           << setw(12) << AM[ih][6] << '\n';
		hstr << "        "
      	  << setw(12) << AM[ih][7]
           << setw(12) << AM[ih][8]
           << setw(12) << AM[ih][9]
			  << setw(12) << AM[ih][10]
           << setw(12) << AM[ih][11]
           << setw(12) << AM[ih][12] << '\n';
 	}

   hstr << "\nReaksi Tummpuan\n";
	hstr << "Titik   AR1         AR2         AR3";
   hstr << "         AR4         AR5         AR6 \n";

	for(int ih=1; ih<=NJ; ih++)
	{
		int J1=(6*ih-5),J2=(6*ih-4),J3=(6*ih-3),
          J4=(6*ih-2),J5=(6*ih-1),J6=(6*ih);
		int N1=JRL[J1]+JRL[J2]+JRL[J3]+JRL[J4]+JRL[J5]+JRL[J6];
		if(N1!=0)
		{
		   hstr << setiosflags(ios::left);
			hstr << setw(8) << ih;

         hstr << setw(12) << AR[J1]
         	  << setw(12) << AR[J2]
              << setw(12) << AR[J3];
			hstr << setw(12) << AR[J4]
         	  << setw(12) << AR[J5]
              << setw(12) << AR[J6] << '\n';
		}
	}
   hstr.close();

   //////////////////////////
   // Cetak Hasil Optimasi //
   //////////////////////////
   hopt << "Hasil Optimasi Beton Bertulang "
        << "Pada Struktur Portal Ruang\n";
   hopt << "Metoda Optimasi : Flexible Polyhedron\n";
   hopt << "Jumlah Variabel Desain : " << JVD << endl;
   hopt << "Jumlah Struktur Desain : " << JSTD << endl;
   hopt << "Oleh Yohan Naftali 1999\n\n";
   hopt << "Nama File Generik : " << fi << endl;
   hopt << "Nama Struktur : " << ISN << endl;
   hopt << "Jumlah batang : " << M << endl;
   hopt << "Jumlah titik kumpul : " << NJ << endl;
   hopt << "Modulus elastisitas tarik/tekan : "
        << (E*1.E-6) << " MPa" << endl;
   hopt << "Modulus elastisitas geser G : "
        << (G*1.E-6) << " MPa" << endl;
   hopt << "Kuat desak Karakteristik Beton : " << FC << " MPa\n";
   hopt << "Kuat tarik baja tulangan lentur : " << FY << " MPa\n";
   hopt << "Kuat tarik baja tulangan sengkang : " << FYS << " MPa\n";

   //////////////////////////
   // Cetak Header Kendala //
   //////////////////////////
   hkdl << "Kendala Pada Struktur\n";
   hkdl << "Oleh Yohan Naftali 1999\n\n";
   hkdl << "Nama File Generik : " << fi << endl;
   hkdl << "Nama Struktur : " << ISN << endl;

   no_struktur=(JSTD-1);
  	for(int oio=0;oio<jum_balok;oio++)
   {
      /////////////////////////////////
		// Memeriksa Kemiringan Batang //
      /////////////////////////////////
      periksa_batang(no_balok[oio]);

   	////////////////////////////
      // Pengisian Elemen Balok //
      ////////////////////////////
      isi_elemen_balok(oio);

      hopt << "-----------------------------------------------------\n";
      hopt << "Balok " << (oio+1) << " " << B << " x " << H << endl;
   	hopt << "Nomor Batang : " << no_balok[oio] << endl;
      hopt << "Panjang Bentang : " << L << " m" << endl;
      hopt << "Beban Total : " << W[no_balok[oio]] << " N/m\n";
      hopt << "Berat Sendiri : " << W_Balok[no_balok[oio]] << " N/m\n";
      hopt << "Momen kiri : "
           << ((MTUM_KI[no_balok[oio]])/(0.8)) << " Nmm" << endl;
      hopt << "Momen tengah : "
           << ((MLAP[no_balok[oio]])/(0.8)) << " Nmm" << endl;
      hopt << "Momen kanan : "
           << ((MTUM_KA[no_balok[oio]])/(0.8)) << " Nmm" << endl;
      hopt << "Geser kiri : "
           << (GESER_KI[no_balok[oio]]/0.8) << " N" << endl;
      hopt << "Geser kanan : "
           << (GESER_KA[no_balok[oio]]/0.8) << " N" << endl;

      /////////////////////
      // Daerah Lapangan //
      /////////////////////
      elemen_lapangan(oio);
      balok lapangan;

      lendutan(no_balok[oio]);

      //////////////////////////
      // Cetak Hasil Optimasi //
      //////////////////////////
      hopt << "Lendutan Tengah Bentang : " << LENDUTAN
           << " mm" << endl;
      hopt << "Lendutan ijin " << LENDUTAN_IJIN << " mm" << endl;
      hopt << "Sengkang " << DIAS << " - " << Jarak_S << endl;
      hopt << "Jarak sengkang maksimum : " << Sref << " mm\n";

      hopt << "\nDaerah Lapangan\n";
      hopt << "Tulangan tarik " << NL1 << " D " << DIA1 << endl;
      hopt << "Tulangan desak " << NL2 << " D " << DIA2 << endl;
      hopt << "Volume beton : " << volume_beton << " m^3" << endl;
      hopt << "Berat Tulangan utama : " << berat_besi << " kg\n";
      hopt << "Berat Tulangan geser : " << berat_sengkang << " kg\n";
      hopt << "Harga balok daerah lapangan : " << harga << endl;
      hopt << "Momen yang membebani : " << MU << " Nmm" << endl;
      hopt << "Momen yang dapat ditahan : " << FMU << " Nmm" << endl;
      hopt << endl;

      ///////////////////
      // Cetak Kendala //
      ///////////////////
      hkdl << "\nBalok " << (oio+1) << endl;
   	hkdl << "Nomor Batang : " << no_balok[oio] << endl;
      hkdl << "Kendala akibat lendutan : "
      	  << kendala_lendutan << endl;
      hkdl << "\nDaerah Lapangan\n";
      hkdl << "Kendala rasio penulangan "
        	  << kendala_rho << endl;
     	hkdl << "Kendala sengkang : "
           << kendala_sb << endl;
		hkdl << "Kendala momen lentur : "
           << kendala_M << endl;


      ////////////////////
      // Daerah Tumpuan //
      ////////////////////
      elemen_tumpuan(oio);
      balok tumpuan;

      //////////////////////////
      // Cetak Hasil Optimasi //
      //////////////////////////    
      hopt << "Daerah Tumpuan\n";
      hopt << "Tulangan desak " << NL2 << " D " << DIA2 << endl;
      hopt << "Tulangan tarik " << NL1 << " D " << DIA1 << endl;
      hopt << "Volume beton : " << volume_beton << " m^3" << endl;
      hopt << "Berat Tulangan utama : " << berat_besi << " kg\n";
      hopt << "Berat Tulangan geser : " << berat_sengkang << " kg\n";
      hopt << "Harga balok pada tumpuan kiri + kanan : " << harga << endl;
      hopt << "Momen yang membebani : " << MU << " Nmm" << endl;
      hopt << "Momen yang dapat ditahan : " << FMU << " Nmm" << endl;
      hopt << endl;

      ///////////////////
      // Cetak Kendala //
      ///////////////////
      hkdl << "\nDaerah Tumpuan\n";
     	hkdl << "Kendala rasio penulangan "
        	  << kendala_rho << endl;
     	hkdl << "Kendala sengkang : "
           << kendala_sb << endl;
		hkdl << "Kendala momen lentur : "
           << kendala_M << endl;
   }

  	////////////////////////////////////////
 	// Menghitung Kendala Dan Harga Kolom //
  	////////////////////////////////////////
  	for(int oio=0;oio<jum_kolom;oio++)
  	{
      /////////////////////////////////
		// Memeriksa Kemiringan Batang //
      /////////////////////////////////
      periksa_batang(no_kolom[oio]);

      ////////////////////////////
      // Pengisian Elemen Balok //
      ////////////////////////////
      isi_elemen_kolom(oio);

      /////////////////////////
      // Membangkitkan Kolom //
      /////////////////////////
      kolom bangkit;

      //////////////////////////
      // Cetak Hasil Optimasi //
      //////////////////////////
      hopt << "-----------------------------------------------------\n";
      hopt << "Kolom " << (oio+1) << " "
           << sisi << " x " << sisi << endl;
      hopt << "Nomor Batang : " << no_kolom[oio] << endl;
      hopt << "Tulangan utama "
           << (4*N_DIA-4) << " D " << DIA << endl;
      hopt << "Jarak antar tulangan : " << jarak_antar_tulangan << " mm\n";
      hopt << "Sengkang " << DIAS << " - " << Jarak_S << endl;
      hopt << "Jarak sengkang maksimum : " << Sref << " mm\n";

      hopt << "Volume beton : " << volume_beton << " m^3" << endl;
      hopt << "Berat Tulangan utama : " << berat_besi << " kg\n";
      hopt << "Berat Tulangan geser : " << berat_sengkang << " kg\n";
      hopt << "Harga kolom : " << harga << endl;

      hopt << "Berat Kolom : " << (-P_Kolom[no_kolom[oio]]) << " N\n";
      hopt << "Gaya aksial : "
           << (PK[no_kolom[oio]]/0.8) << " N" << endl;
      hopt << "Gaya aksial yang dapat ditahan = "
           << FPU << " N\n";
      hopt << "Momen arah X : "
           << (MKX[no_kolom[oio]]/0.8) << " Nmm" << endl;
      hopt << "Momen arah Y : "
           << (MKY[no_kolom[oio]]/0.8) << " Nmm" << endl;
      hopt << "Momen yang dapat ditahan : "
           << FMU << " Nmm\n";
      hopt << "Geser pada kolom : "
           << (GK[no_kolom[oio]]/0.8) << " N" << endl;
      hopt << endl;

      ///////////////////
      // Cetak Kendala //
      ///////////////////
      hkdl << "\nKolom " << (oio+1) << endl;
      hkdl << "Nomor Batang : " << no_kolom[oio] << endl;
      hkdl << "\nKendala gaya : "
           << kendala_gaya << endl;
      hkdl << "Kendala rasio penulangan : "
           << kendala_r << endl;
      hkdl << "Kendala jarak tulangan : "
           << kendala_tul << endl;
      hkdl << "Kendala sengkang : "
           << kendala_sengkang << endl;
      hkdl << "Kendala kelangsingan kolom : "
           << kendala_kelangsingan << endl;
   }
   hopt << "\nHarga Beton Rp." << harga_beton << " /m^3\n";
	hopt << "Harga Besi Rp." << harga_besi << " /kg\n";
	hopt << "Tebal selimut kolom " << selimut_kolom << " mm\n";
   hopt << "Tebal selimut balok " << selimut_balok << " mm\n";
   hopt << "Faktor finalti : " << finalti << endl;
   hopt << "JSTD = (JVD*" << fak_kali << ")+" << fak_plus << endl;
}

//---------------------------------------------------------------------//