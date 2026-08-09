/***********************************************************************/
/* FILE : INOUT.HPP                                                    */
/* KUMPULAN SUBPROGRAM MASUKAN DAN KELUARAN                            */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/

/****************************************/
/* SUBPROGRAM UNTUK MEMASUKAN DATA AWAL */
/****************************************/
void input_data()
{
   cout << "  Nama file input (tanpa ekstensi) = ";
  	cin  >> fi;
   strcpy(finput,fi);
	strcat(finput,".inp");  // input umum
   strcpy(fsisi,fi);
	strcat(fsisi,".isd");   // input sisi diskrit
   strcpy(fdial,fi);
   strcat(fdial,".idl");   // input diameter tulangan lentur
   strcpy(fjtl,fi);
   strcat(fjtl,".ijl");    // input jumlah tulangan lentur
   strcpy(fdias,fi);
   strcat(fdias,".ids");   // input diameter tulangan sengkang
   strcpy(fjts,fi);
   strcat(fjts,".ijs");    // input jarak tulangan sengkang
   int pilih_input;
   do
   {
      clrscr();
   	cout << "  1. Input data umum\n";
   	cout << "  2. Input data diskrit elemen\n";
	 	cout << "  3. Keluar\n";
 		cout << "  Pilihan (1-3) = ";
  		cin  >> pilih_input;
   	if(pilih_input==1)
   	{
   		input_data_umum();
   	}
   	if(pilih_input==2)
   	{
   	   input_data_diskrit();
   	}
   }while(pilih_input!=3);
}

/****************************************/
/* SUBPROGRAM UNTUK MEMASUKAN DATA UMUM */
/****************************************/
void input_data_umum()
{
   int iinp,jinp;
   char uji;

   //////////////////////////
   // Memasukkan Data Umum //
   //////////////////////////
   cout << "  Input data awal\n";
   cout << "  Nama file input (Tanpa Ekstensi) = " << finput << endl;
  	do
	{
		cout << "  Nama Struktur (Tanpa Spasi) = ";
		cin >> ISN;
      cout << "  Jumlah batang = ";
		cin >> M;
      cout << "  Jumlah titik kumpul = ";
		cin >> NJ;
      cout << "  Jumlah titik kumpul yang dikekang = ";
		cin >> NRJ;
      cout << "  Jumlah pengekang tumpuan = ";
		cin >> NR;
		cout << "  Kuat desak Karakteristik Beton (MPa) = ";
      cin >> FC;
      cout << "  Kuat tarik baja tulangan lentur (MPa) = ";
      cin >> FY;
      cout << "  Kuat tarik baja tulangan sengkang (MPa) = ";
      cin >> FYS;

      //////////////////////////////////////////////
      // Hitung Parameter Lainnya Secara Otomatis //
      //////////////////////////////////////////////
      E=(4700.*(sqrt(FC)))*1E6; // (N/mm^2) -> (N/m^2)
      G=(E/2.)*(1.+0.15); // (N/m^2)
      ND=6.*NJ;
		N=ND-NR;

		output_parameter_struktural();
		cout << "\n  Ingin mengulang (y/t) ";
		cin  >> uji;
      if (uji!= 'y' )
		{break;}
	} while(uji== 'y' );

   ////////////////////////////////////////////
   // Memasukkan Data Koordinat Titik Kumpul //
   ////////////////////////////////////////////
   cout << "  Inputkan koordinat titik kumpul (m),"
        << " pilih J = 0 untuk keluar \n";
	do
	{
		cout << "  Titik kumpul J = ";
		cin  >> jinp;
		if ( jinp == 0 )
		{ break ; }
		cout << "  Koordinat\n";
      cout << "  X   Y   Z\n";
		cin  >> X[jinp] >> Y[jinp] >> Z[jinp];
		output_koordinat_titik_kumpul();
	} while(jinp != 0) ;

   ////////////////////////////
   // Mengosongkan Pengekang //
   ////////////////////////////
   for (int jinp=1;jinp<=ND;jinp++)
	{
		JRL[jinp]=0;
	}

   /////////////////////////////////////
   // Memasukkan Data Pengekang Joint //
   /////////////////////////////////////
	cout << endl << "  Inputkan Pengekang titik kumpul,"
                << " masukan 0 untuk keluar \n";
	do
	{
		cout << "  Kekangan ke = ";
		cin  >> iinp;
		if (iinp==0)
		{
      	break ;
      }
		cout << "  Titik kumpul yang dikekang = ";
		cin  >> T_K[iinp];
		cout << "  R (1 = dikekang, 0 = tak dikekang)= ";
		cin  >> JRL[6*T_K[iinp]-5]
           >> JRL[6*T_K[iinp]-4]
           >> JRL[6*T_K[iinp]-3]
           >> JRL[6*T_K[iinp]-2]
           >> JRL[6*T_K[iinp]-1]
           >> JRL[6*T_K[iinp]];
		output_pengekang_titik_kumpul();
	}while (iinp != 0);

   /////////////////////////////////
   // Memasukkan Informasi Batang //
   /////////////////////////////////
   cout << endl << "  Inputkan informasi batang,"
                << " pilih I = 0 untuk keluar \n";
   do
   {
   	cout << "  Indeks batang = ";
      cin  >> iinp;
      if (iinp == 0)
		{ break ; }
      cout << "  titik j = ";
      cin  >> JJ[iinp];
      cout << "  titik k = ";
      cin  >> JK[iinp];
      cout << "  Notasi untuk menunjukan nol atau tidaknya sudut a = ";
		cin  >> IA[iinp];
      if(IA[iinp] != 0)
		{
			cout << "  Sudut alfa tidak 0, masukkan koordinat titik p \n";
			cout << "  Untuk batang " << iinp << endl;
			cout << "  XP   YP   ZP\n";
			cin  >> XP[iinp] >> YP[iinp] >> ZP[iinp];
         cout << endl;
      }
		output_informasi_batang();
	}  while ( iinp != 0 );

   ///////////////////////
   // Penulisan Ke File //
   ///////////////////////
   ofstream tulis(finput);
   tulis << ISN << endl;
   tulis << M << endl;
   tulis << NJ << endl;
   tulis << NRJ << endl;
   tulis << NR << endl;
   tulis << E << endl;
   tulis << G << endl;
   tulis << FC << endl;
   tulis << FY << endl;
   tulis << FYS << endl;
   tulis << ND << endl;
   tulis << N << endl;
   tulis << "[Koordinat]" << endl;
   for(int ktl=1;ktl<=NJ;ktl++)
	{
		tulis << ktl << endl;
		tulis << X[ktl] << endl;
      tulis << Y[ktl] << endl;
      tulis << Z[ktl] << endl;
   }
   tulis << "[Pengekang]" << endl;
   for (int iinp=1;iinp<=NRJ;iinp++)
	{
		tulis << T_K[iinp] << endl;
      tulis << JRL[6*T_K[iinp]-5] << endl;
      tulis << JRL[6*T_K[iinp]-4] << endl;
      tulis << JRL[6*T_K[iinp]-3] << endl;
		tulis << JRL[6*T_K[iinp]-2] << endl;
      tulis << JRL[6*T_K[iinp]-1] << endl;
      tulis << JRL[6*T_K[iinp]] << endl;
	}
   tulis << "[InformasiBatang]" << endl;
   for(int iinp=1;iinp<=M;iinp++)
	{
      tulis << iinp << endl;
      tulis << JJ[iinp] << endl;
		tulis << JK[iinp] << endl;
      tulis << IA[iinp] << endl;
      if(IA[iinp] != 0)
		{
			tulis << XP[iinp] << endl;
         tulis << YP[iinp] << endl;
         tulis << ZP[iinp] << endl;
		}
	}
   tulis.close();
}

/***********************************************************/
/* SUBPROGRAM UNTUK MEMASUKAN DATA DISKRIT VARIABEL DESAIN */
/***********************************************************/
void input_data_diskrit()
{
   char ulang;
   int bsisi,ksisi;
   int bDIA,kDIA;
   int bNL,kNL;
   int bDIAS,kDIAS;
   int bJS,kJS;
   clrscr();

   ///////////////////////////////////////////////////
   // Menggenerasi Data Diskrit Dan Ditulis Ke File //
   ///////////////////////////////////////////////////
   do
   {
      ////////////////////////////////////////////////
      // Menulis Ke File *.isd (Input Sisi Diskrit) //
      ////////////////////////////////////////////////
      ofstream tulis1(fsisi);

      //////////////////////////////////
      // Data Diskrit Lebar Balok (B) //
      //////////////////////////////////
     	cout << "  Jumlah data sisi untuk lebar balok (B) = ";
      cin  >> nsisi_B;
      cout << "  Batas sisi terbawah (mm) = ";
      cin >> bsisi;
      cout << "  Kenaikan sisi (mm) = ";
      cin >> ksisi;
      sisi_d_B[0]=bsisi;
      tulis1 << "[LebarBalok]" << endl;
      tulis1 << nsisi_B << endl;
      tulis1 << sisi_d_B[0] << endl;
      for(int itl1=1;itl1<nsisi_B;itl1++)
      {
      	sisi_d_B[itl1]=sisi_d_B[itl1-1]+ksisi;
         tulis1 << sisi_d_B[itl1] << endl;
      }
      cout << endl;

      ///////////////////////////////////
      // Data Diskrit Tinggi Balok (H) //
      ///////////////////////////////////
      cout << "  Jumlah data sisi untuk tinggi balok (H) = ";
      cin  >> nsisi_H;
      cout << "  Batas sisi terbawah (mm) = ";
      cin >> bsisi;
      cout << "  Kenaikan sisi (mm) = ";
      cin >> ksisi;
      sisi_d_H[0]=bsisi;
      tulis1 << "[TinggiBalok]" << endl;
      tulis1 << nsisi_H << endl;
      tulis1 << sisi_d_H[0] << endl;
      for(int itl1=1;itl1<nsisi_H;itl1++)
      {
      	sisi_d_H[itl1]=sisi_d_H[itl1-1]+ksisi;
         tulis1 << sisi_d_H[itl1] << endl;
      }
      cout << endl;

      /////////////////////////////
      // Data Diskrit Sisi Kolom //
      /////////////////////////////
      cout << "  Jumlah data sisi untuk kolom = ";
      cin  >> nsisi_K;
      cout << "  Batas sisi terbawah (mm) = ";
      cin >> bsisi;
      cout << "  Kenaikan sisi (mm) = ";
      cin >> ksisi;
      sisi_d_K[0]=bsisi;
      tulis1 << "[SisiKolom]" << endl;
      tulis1 << nsisi_K << endl;
      tulis1 << sisi_d_K[0] << endl;
      for(int itl1=1;itl1<nsisi_K;itl1++)
      {
      	sisi_d_K[itl1]=sisi_d_K[itl1-1]+ksisi;
         tulis1 << sisi_d_K[itl1] << endl;
      }
      cout << endl;

      tulis1.close();

      ///////////////////////////////////////////////////
      // Menulis Ke File *.idl (Input Diameter Lentur) //
      ///////////////////////////////////////////////////
      ofstream tulis2(fdial);
      cout << "  Jumlah data diameter tulangan utama = ";
      cin  >> nDIA;
      cout << "  Batas diameter terbawah (mm) = ";
      cin >> bDIA;
      cout << "  Kenaikan diameter (mm) = ";
      cin >> kDIA;
      DIA_d[0]=bDIA;
      tulis2 << "[DiameterTulanganUtama]" << endl;
      tulis2 << nDIA << endl;
      tulis2 << DIA_d[0] << endl;
      for(int itl2=1;itl2<nDIA;itl2++)
      {
      	DIA_d[itl2]=DIA_d[itl2-1]+kDIA;
         tulis2 << DIA_d[itl2] << endl;
      }
      cout << endl;
      tulis2.close();


      //////////////////////////////////////////////////////////
      // Menulis Ke File *.ijl (Input Jumlah tulangan Lentur) //
      //////////////////////////////////////////////////////////
      ofstream tulis3(fjtl);
      cout << "  Jumlah data jumlah tulangan = ";
      cin  >> nNL;
      cout << "  Batas jumlah tulangan terbawah = ";
      cin >> bNL;
      cout << "  Kenaikan jumlah tulangan = ";
      cin >> kNL;
      NL_d[0]=bNL;
      tulis3 << "[JumlahTulanganUtama]" << endl;
      tulis3 << nNL << endl;
      tulis3 << NL_d[0] << endl;
      for(int itl3=1;itl3<nNL;itl3++)
      {
      	NL_d[itl3]=NL_d[itl3-1]+kNL;
         tulis3 << NL_d[itl3] << endl;
      }
      cout << endl;
      tulis3.close();

      //////////////////////////////////////////////////
      // Menulis Ke File *.ids (Input Jarak Sengkang) //
      //////////////////////////////////////////////////
      ofstream tulis4(fdias);
      cout << "  Jumlah data diameter tulangan sengkang = ";
      cin  >> nDIAS;
      cout << "  Batas diameter sengkang terbawah (mm) = ";
      cin >> bDIAS;
      cout << "  Kenaikan diameter sengkang (mm) = ";
      cin >> kDIAS;
      DIAS_d[0]=bDIAS;
      tulis4 << "[DiameterTulanganSengkang]" << endl;
      tulis4 << nDIAS << endl;
      tulis4 << DIAS_d[0] << endl;
      for(int itl4=1;itl4<nDIAS;itl4++)
      {
      	DIAS_d[itl4]=DIAS_d[itl4-1]+kDIAS;
         tulis4 << DIAS_d[itl4] << endl;
      }
      cout << endl;
      tulis4.close();

      //////////////////////////////////////////////////
      // Menulis Ke File *.ijs (Input Jarak Sengkang) //
      //////////////////////////////////////////////////
      ofstream tulis5(fjts);
      cout << "  Jumlah data jarak sengkang = ";
      cin  >> nJS;
      cout << "  Batas jarak sengkang terbawah (mm) = ";
      cin >> bJS;
      cout << "  Kenaikan jarak sengkang (mm) = ";
      cin >> kJS;
      JS_d[0]=bJS;
      tulis5 << "[JarakAntarSengkang]" << endl;
      tulis5 << nJS << endl;
      tulis5 << JS_d[0] << endl;
      for(int itl5=1;itl5<nJS;itl5++)
      {
      	JS_d[itl5]=JS_d[itl5-1]+kJS;
         tulis5 << JS_d[itl5] << endl;
      }
      cout << endl;
      tulis5.close();

      cout << "  Ulang (y/t)";
      cin >> ulang;
   }while(ulang =='y');
}

/****************************************************/
/* SUBPROGRAM UNTUK MEMBACA DATA DARI FILE MASUKKAN */
/****************************************************/
void baca_data()
{
   ifstream baca(finput);
   baca >> ISN;
   baca >> M;
   baca >> NJ;
   baca >> NRJ;
   baca >> NR;
   baca >> E;
   baca >> G;
   baca >> FC;
   baca >> FY;
   baca >> FYS;
   baca >> ND;
   baca >> N;
   baca >> SubName;
   for(int kinp=1;kinp<=NJ;kinp++)
	{
		baca >> kinp;
		baca >> X[kinp];
      baca >> Y[kinp];
      baca >> Z[kinp];
   }
   baca >> SubName;
   for (int iinp=1;iinp<=NRJ;iinp++)
	{
		baca >> T_K[iinp];
      baca >> JRL[6*T_K[iinp]-5];
      baca >> JRL[6*T_K[iinp]-4];
      baca >> JRL[6*T_K[iinp]-3];
		baca >> JRL[6*T_K[iinp]-2];
      baca >> JRL[6*T_K[iinp]-1];
      baca >> JRL[6*T_K[iinp]];
	}
   baca >> SubName;
   for(int iinp=1;iinp<=M;iinp++)
	{
      baca >> iinp;
      baca >> JJ[iinp];
		baca >> JK[iinp];
      baca >> IA[iinp];
      if(IA[iinp] != 0)
		{
			baca >> XP[iinp];
         baca >> YP[iinp];
         baca >> ZP[iinp];
		}
	}
   baca.close();

   ///////////////////////////////////////////////
   // Membaca Data Diskrit Sisi Dari File *.isd //
   ///////////////////////////////////////////////
   ifstream baca1(fsisi);

   //////////////////////////////////////////
   // Membaca Data Diskrit Lebar Balok (B) //
   //////////////////////////////////////////
   baca1 >> SubName;
   baca1 >> nsisi_B;
   for(int iinp=0;iinp<nsisi_B;iinp++)
   {
    	baca1 >> sisi_d_B[iinp];
   }

   ///////////////////////////////////////////
   // Membaca Data Diskrit Tinggi Balok (H) //
   ///////////////////////////////////////////
   baca1 >> SubName;
   baca1 >> nsisi_H;
   for(int iinp=0;iinp<nsisi_H;iinp++)
   {
    	baca1 >> sisi_d_H[iinp];
   }

   /////////////////////////////////////
   // Membaca Data Diskrit Sisi Kolom //
   /////////////////////////////////////
   baca1 >> SubName;
   baca1 >> nsisi_K;
   for(int iinp=0;iinp<nsisi_K;iinp++)
   {
    	baca1 >> sisi_d_K[iinp];
   }

   baca1.close();

   ifstream baca2(fdial);
   baca2 >> SubName;
   baca2 >> nDIA;
   for(int iinp=0;iinp<nDIA;iinp++)
   {
    	baca2 >> DIA_d[iinp];
   }
   baca2.close();

   ifstream baca3(fjtl);
   baca3 >> SubName;
   baca3 >> nNL;
   for(int iinp=0;iinp<nNL;iinp++)
   {
    	baca3 >> NL_d[iinp];
   }
   baca3.close();

   ifstream baca4(fdias);
   baca4 >> SubName;
   baca4 >> nDIAS;
   for(int iinp=0;iinp<nDIAS;iinp++)
   {
    	baca4 >> DIAS_d[iinp];
   }
   baca4.close();

   ifstream baca5(fjts);
   baca5 >> SubName;
   baca5 >> nJS;
   for(int iinp=0;iinp<nJS;iinp++)
   {
    	baca5 >> JS_d[iinp];
   }
   baca5.close();
}

/**************************************************************/
/* SUBPROGRAM UNTUK MENAMPILKAN PARAMETER STRUKTURAL KE LAYAR */
/**************************************************************/
void output_parameter_struktural()
{
	cout << "  Struktur Portal Ruang " << ISN << "\n\n";
	cout << "  Parameter Struktur\n";
   cout << "  Jumlah batang : " << M << endl;
   cout << "  DOF : " << N << endl;
   cout << "  Jumlah joint : " << NJ << endl;
   cout << "  Jumlah pengekang tumpuan : " << NR << endl;
   cout << "  Jumlah titik kumpul yang dikekang : " << NRJ << endl;
   cout << "  Modulus Elastisitas aksial : " << E << " N/m^2\n";
   cout << "  Modulus Geser : " << G << " N/m^2\n\n";
   cout << "  Properti Elemen Material\n";
 	cout << "  Kuat desak beton karakteristik : " << FC << " MPa\n";
   cout << "  Kuat tarik baja tulangan : "  <<  FY << " MPa\n";
   cout << "  Kuat tarik tulangan sengkang : " << FYS  << " MPa\n";
}

/************************************************************/
/* SUBPROGRAM UNTUK MENAMPILKAN KOORDINAT STRUKTUR KE LAYAR */
/************************************************************/
void output_koordinat_titik_kumpul()
{
	cout << "\n  Koordinat Titik Kumpul (m)\n";
	cout << "  Titik     X             Y             Z \n";
	for(int kout=1;kout<=NJ;kout++)
	{
		cout << setiosflags(ios::left);
		cout << "  " << setw(10) << kout
      				 << setw(14) << X[kout]
                   << setw(14) << Y[kout]
                   << setw(14) << Z[kout] << endl;
	}
}

/**********************************************************/
/* SUBPROGRAM UNTUK MENAMPILKAN INFORMASI BATANG KE LAYAR */
/**********************************************************/
void output_informasi_batang()
{
	cout << "\n  Informasi Batang \n";
	cout << "  Batang  JJ      JK      IA\n";
	for(int iout=1;iout<=M;iout++)
	{
		cout << setiosflags(ios::left);
		cout << "  " << setw(8) << iout
      				 << setw(8) << JJ[iout]
                   << setw(8)<< JK[iout]
		             << setw(3) << IA[iout] << endl;
      if(IA[iout] != 0)
		{
			cout << setiosflags(ios::left);
			cout << "  XP = " << setw(12) << XP[iout];
			cout << "  YP = " << setw(12) << YP[iout] << '\t';
			cout << "  ZP = " << setw(12) << ZP[iout] << endl;
			cout << endl;
      }
	}
}

/****************************************************************/
/* SUBPROGRAM UNTUK MENAMPILKAN PENGEKANG TITIK KUMPUL KE LAYAR */
/****************************************************************/
void output_pengekang_titik_kumpul()
{
   cout << "\n  Pengekang Titik Kumpul \n";
	cout << "  Titik     JR1   JR2   JR3   JR4   JR5   JR6\n";
	for (int iout=1;iout<=NRJ;iout++)
	{
		cout << setiosflags(ios::left);
		cout << "  " << setw(10)<<T_K[iout]
      				 << setw(6) << JRL[6*T_K[iout]-5]
                   << setw(6) << JRL[6*T_K[iout]-4]
                   << setw(6) << JRL[6*T_K[iout]-3]
						 << setw(6) << JRL[6*T_K[iout]-2]
      				 << setw(6) << JRL[6*T_K[iout]-1]
                   << setw(6) << JRL[6*T_K[iout]] << endl;
	}
}

/***********************************************************/
/* SUBPROGRAM UNTUK MENAMPILKAN GAYA UJUNG BATANG KE LAYAR */
/***********************************************************/
void output_beban_batang()
{
	cout << "\n  Gaya di Ujung Batang Terkekang Akibat Beban (Nm)\n"
        << "  Batang  AML1        AML2        AML3        "
   	            << "AML4        AML5        AML6\n"
        << "          AML7        AML8        AML9        "
                  << "AML10       AML11       AML12\n";
   for(int kout=1;kout<=M;kout++)
   {
   	cout << setiosflags(ios::left);
   	cout << "  " << setw(8) << kout
      				 << setw(12) << AML[1][kout]
                   << setw(12) << AML[2][kout]
                   << setw(12) << AML[3][kout]
   					 << setw(12) << AML[4][kout]
       	  			 << setw(12) << AML[5][kout]
                   << setw(12) << AML[6][kout] << endl;
      cout << "          "
      			    << setw(12) << AML[7][kout]
                   << setw(12) << AML[8][kout]
                   << setw(12) << AML[9][kout]
   					 << setw(12) << AML[10][kout]
                   << setw(12) << AML[11][kout]
                   << setw(12) << AML[12][kout] << endl;
   }
}

/*****************************************************/
/* SUBPROGRAM UNTUK MENAMPILKAN BEBAN TITIK KE LAYAR */
/*****************************************************/
void output_beban_titik()
{
	cout << "\n  Beban Titik (N)\n"
        << "  Titik   Arah 1      Arah 2      Arah 3      "
   	            << "Arah 4      Arah 5      Arah 6\n";
   for(int kout=1;kout<=NJ;kout++)
   {
   	cout << setiosflags(ios::left);
   	cout << "  " << setw(8) << kout
                   << setw(12) << AJ[((6*kout)-5)]
						 << setw(12) << AJ[((6*kout)-4)]
  						 << setw(12) << AJ[((6*kout)-3)]
                   << setw(12) << AJ[((6*kout)-2)]
						 << setw(12) << AJ[((6*kout)-1)]
						 << setw(12) << AJ[(6*kout)] << endl;
   }
}

//---------------------------------------------------------------------//
