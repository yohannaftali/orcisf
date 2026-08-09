/***********************************************************************/
/* FILE : PEMBEBANAN.HPP                                               */
/* PUSTAKA FUNGSI MENGOLAH DATA PEMBEBANAN KE FILE                     */
/* DITULIS OLEH YOHAN NAFTALI (JULY 1999)                              */
/***********************************************************************/

/*********************************/
/* SUBPROGRAM MEMBACA DATA BEBAN */
/*********************************/
void load_data()
{
   int ibn=0;
   char uji;
   cout << "  Nama file struktur yang akan dikenai beban"
        << " (tanpa ekstensi) = ";
   cin  >> fi;
   strcpy(finput,fi);
   strcat(finput,".inp");
   strcpy(fbeban,fi);
   strcat(fbeban,".bbn");
   baca_data();
   baca_beban();
   cout << "Ingin mengosongkan data beban yang ada (y/t) ? ";
   cin  >> uji;
   if (uji!= 'y' )
	{
   	kosong_beban();
   }

   ///////////////////////
   // Beban Pada Batang //
   ///////////////////////
   cout << "  Inputkan beban merata pada batang,"
        << " pilih batang = 0 untuk keluar\n";
   do
   {
   	cout << "  Batang = ";
      cin  >> ibn;
      if(ibn==0)
		{ break; }
      periksa_batang(ibn);
      cout << "  Panjang = " << EL[ibn] <<endl;
      cout << "  Beban merata (N/m) = "; cin >> W[ibn];
      AML[1][ibn]=0.;
      AML[2][ibn]=W[ibn]*EL[ibn]/2.;
      AML[3][ibn]=0.;
      AML[4][ibn]=0.;
      AML[5][ibn]=0.;
      AML[6][ibn]=W[ibn]*pow(EL[ibn],2)/12.;
      AML[7][ibn]=0.;
      AML[8][ibn]=W[ibn]*EL[ibn]/2.;
      AML[9][ibn]=0.;
      AML[10][ibn]=0.;
      AML[11][ibn]=0.;
      AML[12][ibn]=(-W[ibn])*pow(EL[ibn],2)/12.;
      output_beban_batang();
	} while (ibn!=0);
   ofstream tulis(fbeban);
   tulis << "[BebanBatang]" << endl;
   tulis << M << endl;
   for(int kbn=1;kbn<=M;kbn++)
   {
   	tulis << kbn << endl;
      tulis << W[kbn] << endl;
      tulis << AML[1][kbn] << endl;
      tulis << AML[2][kbn] << endl;
      tulis << AML[3][kbn] << endl;
   	tulis << AML[4][kbn] << endl;
      tulis << AML[5][kbn] << endl;
      tulis << AML[6][kbn] << endl;
      tulis << AML[7][kbn] << endl;
      tulis << AML[8][kbn] << endl;
      tulis << AML[9][kbn] << endl;
   	tulis << AML[10][kbn] << endl;
      tulis << AML[11][kbn] << endl;
      tulis << AML[12][kbn] <<endl;
   }

	//////////////////////
	// Beban Pada Titik //
	//////////////////////
   ibn=0;
  	cout << "  Inputkan beban pada titik kumpul,"
        << " pilih titik = 0 untuk keluar\n";
   do
   {
   	cout << "  Titik = ";
      cin  >> ibn;
      if(ibn==0)
		{
      	break;
      }
      cout << "  Aksi pada arah 1 global = "; cin >> AJ[((6*ibn)-5)];
      cout << "  Aksi pada arah 2 global = "; cin >> AJ[((6*ibn)-4)];
      cout << "  Aksi pada arah 3 global = "; cin >> AJ[((6*ibn)-3)];
      cout << "  Aksi pada arah 4 global = "; cin >> AJ[((6*ibn)-2)];
      cout << "  Aksi pada arah 5 global = "; cin >> AJ[((6*ibn)-1)];
      cout << "  Aksi pada arah 6 global = "; cin >> AJ[(6*ibn)];
		output_beban_titik();
	} while (ibn!=0);
   tulis << "[BebanTitik]" << endl;
   tulis << NJ << endl;
   for(int kbn=1;kbn<=NJ;kbn++)
   {
      tulis << kbn << endl;
      tulis << AJ[((6*kbn)-5)] << endl;
		tulis << AJ[((6*kbn)-4)] << endl;
  		tulis << AJ[((6*kbn)-3)] << endl;
		tulis << AJ[((6*kbn)-2)] << endl;
		tulis << AJ[((6*kbn)-1)] << endl;
		tulis << AJ[(6*kbn)] << endl;
	}
   tulis.close();
}

void kosong_beban()
{
   ////////////////////////////////////
   // Mengosongkan Beban Pada Batang //
   ////////////////////////////////////
   int ibn=0;
   do
   {
   	W[ibn]=0;
      AML[1][ibn]=0.;
      AML[2][ibn]=0.;
      AML[3][ibn]=0.;
      AML[4][ibn]=0.;
      AML[5][ibn]=0.;
      AML[6][ibn]=0.;
      AML[7][ibn]=0.;
      AML[8][ibn]=0.;
      AML[9][ibn]=0.;
      AML[10][ibn]=0.;
      AML[11][ibn]=0.;
      AML[12][ibn]=0.;
      ibn++;
	} while (ibn<=M);

   //////////////////////////////////////////
   // Mengosongkan Beban Pada Titik Kumpul //
   //////////////////////////////////////////
   ibn=0;
   do
   {
      AJ[((6*ibn)-5)]=0.;
		AJ[((6*ibn)-4)]=0.;
		AJ[((6*ibn)-3)]=0.;
		AJ[((6*ibn)-2)]=0.;
		AJ[((6*ibn)-1)]=0.;
		AJ[(6*ibn)]=0.;
      ibn++;
	} while(ibn<=NJ);
}

/******************************************************/
/* SUBPROGRAM UNTUK MEMBACA DATA PEMBEBANAN DARI FILE */
/******************************************************/
void baca_beban()
{
   ifstream baca(fbeban);

   //////////////////////////
   // Membaca Beban Batang //
   //////////////////////////
   baca >> SubName;
   baca >> M;
   for(int kinp=1;kinp<=M;kinp++)
   {
   	baca >> kinp;
      baca >> W[kinp];
      baca >> AML[1][kinp];
      baca >> AML[2][kinp];
      baca >> AML[3][kinp];
   	baca >> AML[4][kinp];
      baca >> AML[5][kinp];
      baca >> AML[6][kinp];
      baca >> AML[7][kinp];
      baca >> AML[8][kinp];
      baca >> AML[9][kinp];
   	baca >> AML[10][kinp];
      baca >> AML[11][kinp];
      baca >> AML[12][kinp];
   }

   /////////////////////////
   // Membaca Beban Titik //
   /////////////////////////
   baca >> SubName;
   baca >> NJ;
   for(int kinp=1;kinp<=NJ;kinp++)
   {
      baca >> kinp;
      baca >> AJ[((6*kinp)-5)];
      baca >> AJ[((6*kinp)-4)];
      baca >> AJ[((6*kinp)-3)];
      baca >> AJ[((6*kinp)-2)];
		baca >> AJ[((6*kinp)-1)];
      baca >> AJ[(6*kinp)];
   }
   baca.close();
   berat_sendiri();
}

/*****************************************/
/* SUBPROGRAM UNTUK MERAKIT VEKTOR BEBAN */
/*****************************************/
void beban()
{
   /////////////////////////////////////////////////////
   // Mengosongkan Vektor Beban Titik Kumpul Gabungan //
   /////////////////////////////////////////////////////
   for(int ib=1;ib<=ND;ib++)
	{
		AC[ID[ib]]=0.;
	}

   /////////////////////////////////////////////////////
   // Mengosongkan Vektor Beban Titik Kumpul Ekivalen //
   /////////////////////////////////////////////////////
   for (int ib=1;ib<=M;ib++)
	{
      periksa_batang(ib);
		if(CXZ<=0.001)
		{
      	continue;
      }
		indeks_batang(ib);
	  	for(int jb=1;jb<=4;jb++)
		{
			AE[IM[(3*jb-2)]]=0.;
			AE[IM[(3*jb-1)]]=0.;
			AE[IM[(3*jb)]]=0.;
      }
   }

   /////////////////////////////////
	// Beban Titik Kumpul Ekivalen //
   /////////////////////////////////
   for (int ib=1;ib<=M;ib++)
	{
      periksa_batang(ib);
      if(CXZ<=0.001)
		{
      	continue;
      }
		indeks_batang(ib);
		for(int jb=1;jb<=4;jb++)
		{
			AE[IM[(3*jb-2)]]=AE[IM[(3*jb-2)]]
         					 -R11[ib]*AML[(3*jb-2)][ib]
         					 -R21[ib]*AML[(3*jb-1)][ib]
               			 -R31[ib]*AML[(3*jb)][ib];
			AE[IM[(3*jb-1)]]=AE[IM[(3*jb-1)]]
         					 -R12[ib]*AML[(3*jb-2)][ib]
         					 -R22[ib]*AML[(3*jb-1)][ib]
               			 -R32[ib]*AML[(3*jb)][ib];
			AE[IM[(3*jb)]]=AE[IM[(3*jb)]]
         				  -R13[ib]*AML[(3*jb-2)][ib]
         				  -R23[ib]*AML[(3*jb-1)][ib]
               		  -R33[ib]*AML[(3*jb)][ib];
		}
   }

   /////////////////////////////////
   // Beban Titik Kumpul Gabungan //
   /////////////////////////////////
   for(int ib=1;ib<=ND;ib++)
	{
		AC[ID[ib]]=AJ[ib]+AE[ib];
	}
}

/**********************************************/
/* SUBPRROGRAM MENGHITUNG BERAT SENDIRI BALOK */
/**********************************************/
void berat_sendiri()
{
	js_balok=0;
   js_kolom=0;
	for(int ibs=1;ibs<=M;ibs++)
   {
      /////////////////////////////////
      // Memeriksa Kemiringan Batang //
      /////////////////////////////////
      periksa_batang(ibs);

      /////////////////////////
      // Batang Non Vertikal //
      /////////////////////////
      if(CXZ>0.001)
      {
         periksa_batang(ibs);
         // (mm) -> (m)
      	b[ibs]=isi(var_b[no_struktur][0+(12*js_balok)],sisi_d_B);
   		h[ibs]=isi(var_b[no_struktur][1+(12*js_balok)],sisi_d_H);
      	W_Balok[ibs]=24000.*b[ibs]*h[ibs]*1.E-6; // (N/m)
         W[ibs]+=W_Balok[ibs]; // (N/m)
			AML[2][ibs]+=W_Balok[ibs]*EL[ibs]/2.;
      	AML[6][ibs]+=W_Balok[ibs]*pow(EL[ibs],2)/12.;
      	AML[8][ibs]+=W_Balok[ibs]*EL[ibs]/2.;
      	AML[12][ibs]+=(-W_Balok[ibs])*pow(EL[ibs],2)/12.;
         js_balok++;
      }

      /////////////////////
      // Batang Vertikal //
      /////////////////////
      else
      {
         periksa_batang(ibs);
      	b[ibs]=isi(var_k[no_struktur][0+(5*js_kolom)],sisi_d_K);
         h[ibs]=b[ibs];
         P_Kolom[ibs]=(-24000.)*b[ibs]*h[ibs]*EL[ibs]*1.E-6; // (N)

         AJ[((6*JJ[ibs])-4)]+=P_Kolom[ibs];

         js_kolom++;
      }
   }
}

//---------------------------------------------------------------------//
