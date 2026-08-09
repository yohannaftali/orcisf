/***********************************************************************/
/* FILE : TAMPILAN.HPP			                  							  */
/* KUMPULAN SUBPROGRAM PENANGANAN TAMPILAN KE LAYAR                    */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/

/*******************************************/
/* SUBPROGRAM UNTUK MENAMPILKAN MENU UTAMA */
/*******************************************/
void menu_utama()
{
   int pilih;
   do
   {
      textbackground(1);
      textcolor(LIGHTGREEN);
      clrscr();
      about();
   	cout << "  1. Input data awal ke file\n";
      cout << "  2. Input data beban ke file\n";
      cout << "  3. Melihat isi file input\n";
   	cout << "  4. Mengoptimasi struktur\n";
      cout << "  5. Keluar\n";
   	cout << "  Pilihan (1-5) = ";
      struct time t;

      	gettime(&t);
      	textcolor(YELLOW);
      	gotoxy(61,6);
      	cprintf("Time : %2d:%02d:%02d.%02d\n",
         	t.ti_hour, t.ti_min, t.ti_sec, t.ti_hund);
         gotoxy(19,12);
         cin  >> pilih;

   	if(pilih==1)
      {
         input_data();
      }
      if(pilih==2)
      {
      	load_data();   // Memasukan data beban
         cout << "  Data beban sudah diisi...\n";
      }
      if(pilih==3)
      {
         cout << "  Nama file input yang akan dibaca = ";
         cin  >> fi;
         strcpy(finput,fi);
   		strcat(finput,".inp");
         strcpy(fbeban,fi);
   		strcat(fbeban,".bbn");
         baca_data();
         baca_beban();
         do
         {
            clrscr();
            about();
	         cout << "  1. Data umum\n";
   	      cout << "  2. Koordinat titik kumpul\n";
      	   cout << "  3. Informasi batang\n";
         	cout << "  4. Pengekang titik kumpul\n";
            cout << "  5. Gaya ujung batang terkekang\n";
            cout << "  6. Beban pada joint\n";
            cout << "  7. Keluar ke menu utama\n";
	         cout << "  Pilihan (1-7) = ";
   	      cin  >> pilih;
            clrscr();
      	   if(pilih==1)
            {
            	output_parameter_struktural();
            }
        	 	if(pilih==2)
            {
            	output_koordinat_titik_kumpul();
            }
     		   if(pilih==3)
            {
            	output_informasi_batang();
            }
         	if(pilih==4)
            {
            	output_pengekang_titik_kumpul();
            }
            if(pilih==5)
            {
            	output_beban_batang();
            }
            if(pilih==6)
            {
            	output_beban_titik();
            }
            if(pilih==7)
            {
            	cout << "  Anda keluar ke Menu Utama\n";
         	}
            cout << "\n  Tekan Sembarang Tombol\n";
            getch();
      	}while(pilih!=7);
      }
   	if(pilih==4)
      {
         cout << "  Nama file stuktur (tanpa ekstensi) = ";
         cin  >> fi;
      	strcpy(finput,fi);
         strcat(finput,".inp");     // Nama file generik
         strcpy(fsisi,fi);
			strcat(fsisi,".isd");   	// input sisi diskrit
		   strcpy(fdial,fi);
		   strcat(fdial,".idl");   	// input diameter tulangan lentur
		   strcpy(fjtl,fi);
		   strcat(fjtl,".ijl");    	// input jumlah tulangan lentur
		   strcpy(fdias,fi);
		   strcat(fdias,".ids");   	// input diameter tulangan sengkang
		   strcpy(fjts,fi);
		   strcat(fjts,".ijs");    	// input jarak tulangan sengkang
         strcpy(fbeban,fi);
         strcat(fbeban,".bbn");     // input beban pada batang
         strcpy(fhistory,fi);
         strcat(fhistory,".his");  	// Keluaran dari riwayat optimasi
         strcpy(foptimasi,fi);
         strcat(foptimasi,".opt");  // Keluaran dari hasil optimasi
         strcpy(fstruktur,fi);
         strcat(fstruktur,".str");  // Keluaran dari hasil analisa struktur
         strcpy(fkendala,fi);
         strcat(fkendala,".kdl");   // Keluaran kendala pada struktur
         strcpy(finformasi,fi);
         strcat(finformasi,".inf"); // Keluaran informasi masukkan


         cout << "  file input = " << finput << endl;
         cout << "  file beban = " << fbeban << endl;
         cout << "  Harga Beton (Rp./m^3) = ";
   		cin >> harga_beton;
		   cout << "  Harga Besi (Rp./kg) = ";
		   cin >> harga_besi;
		   cout << "  Tebal selimut kolom (minimum 40 mm) = ";
		   cin >> selimut_kolom;
         cout << "  Tebal selimut balok (minimum 40 mm) = ";
		   cin >> selimut_balok;
         cout << "  Faktor penalti = ";
         cin >> finalti;
         cout << "  Iterasi maksimum = ";
         cin >> j_iterasi_mak;
         cout << "  Faktor penambah jumlah variabel desain = ";
         cin >> fak_plus;
         do
         {
         	cout << "  Faktor pengali jumlah variabel desain = ";
         	cin >> fak_kali;
            if(fak_kali<=0)
            {
            	"  Faktor pengali minimal diisi 1 !\n";
            }
         }while(fak_kali<=0);
         optimasi();
      }
      if(pilih==5)
      {
         cout << "\n                       Anda keluar dari program\n";
      }
   } while(pilih!=5);
}

/**************************************************/
/* SUBPROGRAM UNTUK MENAMPILKAN INFORMASI PROGRAM */
/**************************************************/
void about()
{
  	cout << "****************************************";
   cout << "****************************************";
	cout << "          "
   << "Program Optimasi Beton Bertulang Pada Struktur Portal Ruang\n";
	cout << "                            "
   << "Oleh : Yohan Naftali\n";
	cout << "              		   	  "
   << "7712/TS\n";
   cout << "****************************************";
	cout << "****************************************\n";
}

//---------------------------------------------------------------------//
