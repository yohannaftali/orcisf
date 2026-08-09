/***********************************************************************/
/* FILE : STRUKTUR.HPP                                                 */
/* KUMPULAN SUBPROGRAM UNTUK MENGHITUNG STRUKTUR                       */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/* DIKEMBANGKAN DARI WEAVER & GERE                                     */
/***********************************************************************/

/************************************************************/
/* SUBPROGRAM MENGHITUNG GAYA DAN PERPINDAHAN PADA STRUKTUR */
/************************************************************/
void struktur()
{
   kekakuan_batang();
	banfac(N,NB,SFF);
  	beban();
  	bansol(N,NB,SFF,AC,DF);
  	hasil();
}

/*********************************/
/* SUBPROGRAM MENGHITUNG INERSIA */
/*********************************/
void inersia()
{
	js_balok=0;
   js_kolom=0;
	for (int iin=1;iin<=M;iin++)
   {
      /////////////////////////////////
      // Memeriksa Kemiringan Batang //
      /////////////////////////////////
      periksa_batang(iin);

      /////////////////////////
      // Batang Non Vertikal //
      /////////////////////////
      if(CXZ>0.001)
      {
         // (mm) -> (m)
      	b[iin]=isi(var_b[no_struktur][0+(12*js_balok)],sisi_d_B)/1000.;
   		h[iin]=isi(var_b[no_struktur][1+(12*js_balok)],sisi_d_H)/1000.;
         js_balok++;
      }

      /////////////////////
      // Batang Vertikal //
      /////////////////////
      else
      {
         // (mm) -> (m)
      	b[iin]=isi(var_k[no_struktur][0+(5*js_kolom)],sisi_d_K)/1000.;
         h[iin]=b[iin];
         js_kolom++;
      }

      ///////////////////////////
      // Menghitung Luas (m^2) //
      ///////////////////////////
      AX[iin]=b[iin]*h[iin];

      /////////////////////////////////
      // Menghitung Konstanta Puntir //
      /////////////////////////////////
   	if(b[iin]<=h[iin])
   	{
     		XI[iin]=((((1./3.)-(0.21*b[iin]/h[iin]*
                     ((1.-(pow(b[iin],4)/(12.*(pow(h[iin],4)))))
         			))))*(h[iin])*(pow(b[iin],3)));
   	}
   	else
      {
         ////////////////////////////////////////////////////////////////////
         // Formula Diambil Dari Buku Roark's Formulas for Stress & Strain //
         // Warren C. Young 1989 halaman 348                               //
         ////////////////////////////////////////////////////////////////////
         float _a=0.5*b[iin];
         float _b=0.5*h[iin];
      	XI[iin]=_a*(pow(_b,3))*
         		 (
                	(16./3.)-(3.36*_b/_a)*(1.-(pow(_b,4))/(12.*pow(_a,4)))
                );
      }

      ///////////////////
      // Momen Inersia //
      ///////////////////
   	YI[iin]=h[iin]*(pow(b[iin],3))/12.;
   	ZI[iin]=b[iin]*(pow(h[iin],3))/12.;
   }
}

/***********************************************/
/* SUBPROGRAM MERAKIT MATRIK KEKAKUAN STRUKTUR */
/***********************************************/
void kekakuan_batang()
{
   ///////////////////////
   // Inisialisasi Awal //
   ///////////////////////
   NB=0.;
   IR=0;
   IC=0;
   for (int ir=1;ir<=M;ir++)
   {
   	R11[ir]=0.;R12[ir]=0.;R13[ir]=0.;
		R21[ir]=0.;R22[ir]=0.;R23[ir]=0.;
		R31[ir]=0.;R32[ir]=0.;R33[ir]=0.;
   }

   /////////////////////////////
   // Membentuk Matrik Rotasi //
   /////////////////////////////
	for (int ir=1;ir<=M;ir++)
	{
		if ((6*(abs(JK[ir]-JJ[ir])+1))>NB)
      {
      	NB=(6*(abs(JK[ir]-JJ[ir])+1));
      }
		periksa_batang(ir);
      if(IA[ir]!=0)
		{
			XPS=XP[ir]-(X[JJ[ir]]);
			YPS=YP[ir]-(Y[JJ[ir]]);
			ZPS=ZP[ir]-(Z[JJ[ir]]);
		}

      ///////////////////////////////////////////////////
      // Membentuk Matrik Rotasi Untuk Batang Vertikal //
      ///////////////////////////////////////////////////
		if(CXZ<=0.001)
		{
			R11[ir]=0.;   	R12[ir]=Cy;  R13[ir]=0.;
         R21[ir]=(-Cy); R22[ir]=0.;  R23[ir]=0.;
         R31[ir]=0.;   	R32[ir]=0.;  R33[ir]=1.;
         if(IA[ir]==0)
			{
         	continue;
         }
			COSA=(-XPS*Cy)/(sqrt(XPS*XPS+ZPS*ZPS));
			SINA=(-ZPS)/(sqrt(XPS*XPS+ZPS*ZPS));
         R21[ir]=(-Cy*COSA);
         R23[ir]=SINA;
			R31[ir]=Cy*SINA;
         R33[ir]=COSA;
			continue;
		}

      ///////////////////////////////////////////////////////
      // Membentuk Matrik Rotasi Untuk Batang Non Vertikal //
      ///////////////////////////////////////////////////////
		R11[ir]=CX;         		R12[ir]=Cy;    R13[ir]=CZ;
		R21[ir]=(-CX*Cy)/CXZ; 	R22[ir]=CXZ;   R23[ir]=(-Cy*CZ)/CXZ;
		R31[ir]=(-CZ)/CXZ;    	R32[ir]=0.;    R33[ir]=CX/CXZ;
		if(IA[ir]==0)
		{
      	continue;
      }
      YPG=R21[ir]*XPS+R22[ir]*YPS+R23[ir]*ZPS;
		ZPG=R31[ir]*XPS+R32[ir]*YPS+R33[ir]*ZPS;
		COSA=YPG/(sqrt(YPG*YPG+ZPG*ZPG));
		SINA=ZPG/(sqrt(YPG*YPG+ZPG*ZPG));
		R21[ir]=((-CX*Cy*COSA)-CZ*SINA)/CXZ; R22[ir]=CXZ*COSA;
      R23[ir]=((-Cy*CZ*COSA)+CX*SINA)/CXZ;
      R31[ir]=(CX*Cy*SINA-CZ*COSA)/CXZ;  R32[ir]=(-CXZ*SINA);
      R33[ir]=(Cy*CZ*SINA+CX*COSA)/CXZ;
   }

   /////////////////////////////////////
   // Inisialisasi Akumulator N1 = 0  //
   /////////////////////////////////////
   int N1=0;

   ////////////////////////////////
   // Mengisi Indeks Perpindahan //
   ////////////////////////////////
   for(int ir=1;ir<=ND;ir++)
	{
		N1+=JRL[ir];
		if((JRL[ir])<=0)
		{
			ID[ir]=ir-N1;
			continue ;
		}
		ID[ir]=N+N1;
	}

   //////////////////////////////////////
   // Mengosongkan Matrik Kekakuan SFF //
   //////////////////////////////////////
   for(int ir=1;ir<=N;ir++)
   {
   	for(int jr=1;jr<=NB;jr++)
      {
      	SFF[ir][jr]=0.;
      }
	}

   ////////////////////////////////////////
   // Pembentukan Matrik Kekakuan Batang //
   ////////////////////////////////////////
   for(int ir=1;ir<=M;ir++)
	{
      periksa_batang(ir);

      ////////////////////////////////////////////////////
      // Mengisi Matrik Kekakuan Batang Pada Arah Lokal //
      ////////////////////////////////////////////////////
		isi_matrik_kekakuan(ir);

      ////////////////////////////////////////////////////////////////
      // Membentuk Matrik Kekakuan Batang Untuk Sumbu Arah Struktur //
      ////////////////////////////////////////////////////////////////
      for(int jr=1;jr<=4;jr++)
		{
			for(int kr=((3*jr)-2);kr<=12;kr++)
			{
				SMS[((3*jr)-2)][kr]=R11[ir]*SMRT[((3*jr)-2)][kr]
            			          +R21[ir]*SMRT[((3*jr)-1)][kr]
                               +R31[ir]*SMRT[(3*jr)][kr];
				SMS[((3*jr)-1)][kr]=R12[ir]*SMRT[((3*jr)-2)][kr]
            			          +R22[ir]*SMRT[((3*jr)-1)][kr]
                               +R32[ir]*SMRT[(3*jr)][kr];
				SMS[(3*jr)][kr]=R13[ir]*SMRT[((3*jr)-2)][kr]
            			      +R23[ir]*SMRT[((3*jr)-1)][kr]
                           +R33[ir]*SMRT[(3*jr)][kr];
			}
		}

      //////////////////////////////////////////////
      // Mengisi Vektor Indeks Perpindahan Batang //
      //////////////////////////////////////////////
		indeks_batang(ir);

      /////////////////////////////////////////////////////
      // Mengambil Matrik Kekakuan                       //
      // Untuk Perpindahan Titik Kumpul Yang Bebas (SFF) //
      /////////////////////////////////////////////////////
      for(int jr=1;jr<=MD;jr++)
		{
			if(JRL[IM[jr]]==0)
			{
				for(int kr=jr;kr<=MD;kr++)
				{
               if(JRL[IM[kr]]==0)
					{
						IR=ID[IM[jr]];
                  IC=ID[IM[kr]];
						if(IR>=IC)
						{
							int ITEM=IR;
							IR=IC;
							IC=ITEM;
						}
                  IC=(IC-IR+1);
                  SFF[IR][IC]=((SFF[IR][IC])+(SMS[jr][kr]));
					}
				}
			}
   	}
	}
}

/*****************************************************/
/* SUBPROGRAM UNTUK MENGOLAH HASIL HITUNGAN STRUKTUR */
/*****************************************************/
void hasil()
{
   /////////////////////////////////////////
   // Inisialisasi Vektor Perpindahan = 0 //
   /////////////////////////////////////////
   int ihs=N+1;
   for(int jhs=1;jhs<=ND;jhs++)
	{
		if(JRL[(ND-jhs+1)]==0)
      {
      	ihs=(ihs-1);
         DJ[(ND-jhs+1)]=0.;
      }
		else
      {
      	DJ[(ND-jhs+1)]=0.;
      }
	}

   ///////////////////////////////////////////////////
   // Mengisi Vektor Perpindahan Semua Titik Kumpul //
   // Dalam Arah Sumbu Global                       //
   ///////////////////////////////////////////////////
   ihs=N+1;
	for(int jhs=1;jhs<=ND;jhs++)
	{
		if(JRL[(ND-jhs+1)]==0)
      {
      	ihs=(ihs-1);
         DJ[(ND-jhs+1)]=DF[ihs];
      }
		else
      {
      	DJ[(ND-jhs+1)]=0.;
      }
	}

   ////////////////////////
   // Gaya Pada Struktur //
   ////////////////////////
   for(int ih=1;ih<=M;ih++)
	{
      periksa_batang(ih);

      //////////////////////////////////////////////////////////
      // Mengisi Matrik Kekakuan Batang Pada Arah Sumbu Lokal //
      //////////////////////////////////////////////////////////
		isi_matrik_kekakuan(ih);

      //////////////////////////////////////////////
      // Mengisi Vektor Indeks Perpindahan Batang //
      //////////////////////////////////////////////
      indeks_batang(ih);

      ////////////////////////////////////////////
      // Mengisi Vektor Gaya Ujung Batang Akhir //
      ////////////////////////////////////////////
     	for(int jh=1;jh<=MD;jh++)
		{
         ///////////////////////////////////////////////////////////
         // Inisialisasi Gaya Ujung Batang Akibat Perpindahan = 0 //
         ///////////////////////////////////////////////////////////
			AMD[jh]=0.;

         /////////////////////////////////////////
         // Mengisi Vektor Gaya Di Ujung Batang //
         // Akibat Perpindahan Titik Kumpul     //
         /////////////////////////////////////////
			for(int kh=1;kh<=MD;kh++)
			{
				AMD[jh]=AMD[jh]+SMRT[jh][kh]*DJ[IM[kh]];
			}
         AM[ih][jh]=AML[jh][ih]+AMD[jh];
		}

      periksa_batang(ih);

      ////////////////////////////////////////////
      // Menyimpan Gaya Untuk Perhitungan Balok //
      ////////////////////////////////////////////
      if(CXZ>0.001)
      {
         //////////////////////////////////
         // Momen di tengah bentang (Nm) //
         //////////////////////////////////
         MLAP[ih]=(-AM[ih][6])
         		  +(0.125*W[ih]*pow(EL[ih],2));

         /////////////////////////////
         // Momen Pada Tumpuan (Nm) //
         /////////////////////////////
         MTUM_KI[ih]=(-AM[ih][6]);
         MTUM_KA[ih]=AM[ih][12];

         ////////////////////
         // Gaya Geser (N) //
         ////////////////////
         GESER_KI[ih]=AM[ih][2];
         GESER_KA[ih]=(-AM[ih][8]);
      }

      /////////////////////////////////////////////////////
      // Menyimpan Gaya Untuk Perhitungan Kolom Biaksial //
      /////////////////////////////////////////////////////
      else
      {
         ///////////////////////////////
         // Gaya Aksial Dan Momen (N) //
         ///////////////////////////////
         PK[ih]=fabs(AM[ih][7]);
         MKX[ih]=fabs(AM[ih][10]);
	      MKY[ih]=fabs(AM[ih][12]);

         ////////////////////
         // Gaya Geser (N) //
         ////////////////////
         if(fabs(AM[ih][8])>fabs(AM[ih][9]))
         {
         	GK[ih]=fabs(AM[ih][8]);
         }
         else
         {
         	GK[ih]=fabs(AM[ih][9]);
         }
      }

      ////////////////////////////////////////
      // Inisialisasi Vektor Reaksi Tumpuan //
      ////////////////////////////////////////
      for(int jh=1; jh<=4; jh++)
		{
			if(JRL[IM[((3*jh)-2)]]==1)
         {
         	AR[IM[((3*jh)-2)]]=0.;
         }
			if(JRL[IM[((3*jh)-1)]]==1)
         {
         	AR[IM[((3*jh)-1)]]=0.;
         }
			if(JRL[IM[(3*jh)]]==1)
         {
         	AR[IM[(3*jh)]]=0.;
         }
      }

      ////////////////////
      // Reaksi Tumpuan //
      ////////////////////
		for(int jh=1; jh<=4; jh++)
		{
			int J1=((3*jh)-2),J2=((3*jh)-1),J3=(3*jh);
			int I1=IM[J1],I2=IM[J2],I3=IM[J3];
			if(JRL[I1]==1)
			{
				AR[I1]=AR[I1]
            		+R11[ih]*AMD[J1]
                  +R21[ih]*AMD[J2]
                  +R31[ih]*AMD[J3];
			}
			if(JRL[I2]==1)
			{
				AR[I2]=AR[I2]
            		+R12[ih]*AMD[J1]
                  +R22[ih]*AMD[J2]
                  +R32[ih]*AMD[J3];
			}
			if(JRL[I3]==1)
			{
				AR[I3]=AR[I3]
            		+R13[ih]*AMD[J1]
                  +R23[ih]*AMD[J2]
                  +R33[ih]*AMD[J3];
			}
		}
	}

   /////////////////////////////////////////////////////
   // Menghitung Reaksi Tumpuan Untuk Titik Terkekang //
   /////////////////////////////////////////////////////
   for(int ih=1;ih<=ND;ih++)
	{
		if(JRL[ih]==0)
		{
      	continue;
      }
		AR[ih]=AR[ih]-AJ[ih]-AE[ih];
	}
}

/************************************************/
/* SUBPROGRAM UNTUK MEMERIKSA KEMIRINGAN BATANG */
/************************************************/
void periksa_batang(int perb)
{
	EL[perb] = sqrt(
   			 (pow(((X[JK[perb]])-(X[JJ[perb]])),2))
   			+(pow(((Y[JK[perb]])-(Y[JJ[perb]])),2))
            +(pow(((Z[JK[perb]])-(Z[JJ[perb]])),2)));
	CX=((X[JK[perb]])-(X[JJ[perb]]))/EL[perb];
	Cy=((Y[JK[perb]])-(Y[JJ[perb]]))/EL[perb];
	CZ=((Z[JK[perb]])-(Z[JJ[perb]]))/EL[perb];
   CXZ = fabs(sqrt(CX*CX + CZ*CZ));
}

/****************************************************************/
/* SUBPROGRAM UNTUK MENGHITUNG VEKTOR INDEKS PERPINDAHAN BATANG */
/****************************************************************/
void indeks_batang(int ib)
{
   IM[1] =((6.*JJ[ib])-5.);  	IM[2] =((6.*JJ[ib])-4.);
	IM[3] =((6.*JJ[ib])-3.);  	IM[4] =((6.*JJ[ib])-2.);
	IM[5] =((6.*JJ[ib])-1.);  	IM[6] =(6.*JJ[ib]);
	IM[7] =((6.*JK[ib])-5.);  	IM[8] =((6.*JK[ib])-4.);
	IM[9] =((6.*JK[ib])-3.);  	IM[10]=((6.*JK[ib])-2.);
	IM[11]=((6.*JK[ib])-1.); 	IM[12]=(6.*JK[ib]);
}

/********************************************************************/
/* SUBPROGRAM UNTUK MENGISI MATRIK KEKAKUAN BATANG PADA SUMBU LOKAL */
/********************************************************************/
void isi_matrik_kekakuan(int imk)
{
   ///////////////////////////////////////////////
   // Mengosongkan Matrik Kekakuan Batang Lokal //
   ///////////////////////////////////////////////
   for(int is=1;is<=12;is++)
	{
   	for(int js=1;js<=12;js++)
      {
      	SM[is][js]=0.;
      }
   }

   //////////////////////////////////
   // Menghitung Delapan Konstanta //
   //////////////////////////////////
	float SCM1A=E*AX[imk]/EL[imk];
   float SCM1B=G*XI[imk]/EL[imk];
	float SCM2Y=4.0*E*YI[imk]/EL[imk];
   float SCM3Y=1.5*SCM2Y/EL[imk];
	float SCM4Y=2.0*SCM3Y/EL[imk];
   float SCM2Z=4.0*E*ZI[imk]/EL[imk];
	float SCM3Z=1.5*SCM2Z/EL[imk];
   float SCM4Z=2.0*SCM3Z/EL[imk];

   //////////////////////////////////////////////////
   // Mengisi Segitiga Atas Matrik Kekakuan Batang //
   //////////////////////////////////////////////////
   SM[1][1]=SCM1A;            	SM[1][7]=(-SCM1A);
	SM[2][2]=SCM4Z;            	SM[2][6]=SCM3Z;
	SM[2][8]=(-SCM4Z);           	SM[2][12]=SCM3Z;
	SM[3][3]=SCM4Y;            	SM[3][5]=(-SCM3Y);
	SM[3][9]=(-SCM4Y);           	SM[3][11]=(-SCM3Y);
	SM[4][4]=SCM1B;            	SM[4][10]=(-SCM1B);
	SM[5][5]=SCM2Y;            	SM[5][9]=SCM3Y;
	SM[5][11]=SCM2Y/2.0;       	SM[6][6]=SCM2Z;
	SM[6][8]=(-SCM3Z);           	SM[6][12]=SCM2Z/2.0;
	SM[7][7]=SCM1A;            	SM[8][8]=SCM4Z;
	SM[8][12]=(-SCM3Z);          	SM[9][9]=SCM4Y;
	SM[9][11]=SCM3Y;           	SM[10][10]=SCM1B;
	SM[11][11]=SCM2Y;          	SM[12][12]=SCM2Z;

   ////////////////////////////////////////////////
   // Mengisi Segitiga Bawah Matrik Yang Simetri //
   ////////////////////////////////////////////////
   for(int is=1;is<=11;is++)
	{
		for(int js=is+1;js<=12;js++)
		{
			SM[js][is]=SM[is][js];
		}
	}

   ///////////////////////////////////////////////
   // Mengalikan Matrik SM Dengan Matrik Rotasi //
   ///////////////////////////////////////////////
   for(int is=1;is<=4;is++)
	{
		for(int js=1;js<=12;js++)
		{
			SMRT[js][((3*is)-2)]=SM[js][((3*is)-2)]*R11[imk]
         						  +SM[js][((3*is)-1)]*R21[imk]
                     		  +SM[js][(3*is)]*R31[imk];
			SMRT[js][((3*is)-1)]=SM[js][((3*is)-2)]*R12[imk]
         						  +SM[js][((3*is)-1)]*R22[imk]
                             +SM[js][(3*is)]*R32[imk];
			SMRT[js][(3*is)]=SM[js][((3*is)-2)]*R13[imk]
         					 +SM[js][((3*is)-1)]*R23[imk]
                     	 +SM[js][(3*is)]*R33[imk];
		}
	}
}

//----------------------------------------------------------------------//
