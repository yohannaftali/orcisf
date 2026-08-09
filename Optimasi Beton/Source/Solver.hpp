/***********************************************************************/
/* FILE : SOLVER.HPP                                                   */
/* SUBPROGRAM UNTUK MENYELESAIKAN PERSAMAAN                            */
/* DENGAN METODA CHOLESKI YANG DIMODIFIKASI                            */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/

/*********************************************************/
/* SUBPROGRAM UNTUK FAKTORISASI MATRIK SIMETRIS BERJALUR */
/* DENGAN PENDEKATAN CHOLESKI YANG DIMODIFIKASI          */
/*********************************************************/
void banfac(int N,int NB,float A_SFF[][mak])
{
   int J2;
   float SUM;
   float TEMP;
   if((A_SFF[1][1])>0)
   {
      for(int jbf=2;jbf<=N;jbf++)
	   {
	   	J2=(jbf-NB+1);
			if(J2<1)
	      {
         	J2=1;
         }
	      if((jbf-1)!=1)
	      {
	      	for(int ibf=2;ibf<=(jbf-1);ibf++)
		      {
	   	      if((ibf-1)>=J2)
   	   	   {
      	   		SUM=A_SFF[ibf][jbf-ibf+1];
		         	for(int kbf=J2;kbf<=(ibf-1);kbf++)
			         {
   			      	SUM=SUM-A_SFF[kbf][ibf-kbf+1]
                        *A_SFF[kbf][jbf-kbf+1];
      			   }
	         		A_SFF[ibf][jbf-ibf+1]=SUM;
					}
					else
					{
               	continue;
               }
	   	   }
	      }
    	  	SUM=A_SFF[jbf][1];
      	for(int kbf=J2;kbf<=(jbf-1);kbf++)
	      {
   	   	TEMP=A_SFF[kbf][jbf-kbf+1]/A_SFF[kbf][1];
      	   SUM=SUM-TEMP*A_SFF[kbf][jbf-kbf+1];
         	A_SFF[kbf][jbf-kbf+1]=TEMP;
	      }
   	   if(SUM<=0)
      	{
         	break;
         }
	      A_SFF[jbf][1]=SUM;
   	}
   }
   else
   {
      clrscr();
   	cout << "Tidak positif tentu !!!\n";
      cout << "Struktur tak stabil - GAGAL dibangkitkan !!\n\a";
      cout << "Periksa data struktur anda \n";
      cout << "Tekan sembarang tombol, program akan dihentikan \n";
      getch();
      exit(1);
   }
}

/****************************************************************/
/* SUBPROGRAM MENGOLAH MATRIK BERJALUR DARI SUBPROGRAM BANFAC() */
/****************************************************************/
void bansol(int N,int NB,float U_SFF[][mak]
	         ,float B_AC[mak],float X_DF[mak])
{
   float SUM;

   /////////////////////////////////////////////////////
   // Inisialisasi Perpindahan Titik Kumpul Bebas = 0 //
   /////////////////////////////////////////////////////
   for(int ins=1;ins<=N;ins++)
	{
		X_DF[ins]=0.;
	}

   for(int ibsl=1;ibsl<=N;ibsl++)
	{
		int jbns=(ibsl-NB+1);
		if (ibsl<=NB)
		{
      	jbns=1;
      }
		SUM=B_AC[ibsl];
		if(jbns<=(ibsl-1))
		{
			for(int kbns=jbns;kbns<=(ibsl-1);kbns++)
			{
				SUM=(SUM-((U_SFF[kbns][ibsl-kbns+1])*(X_DF[kbns])));
			}
		}
		X_DF[ibsl] = SUM;
	}

	for(int ibsl=1;ibsl<=N;ibsl++)
	{
		X_DF[ibsl]=((X_DF[ibsl])/(U_SFF[ibsl][1]));
	}

	for(int I1=1;I1<=N;I1++)
	{
		int ibsl=(N-I1+1);
		int jbns=(ibsl+NB-1);
		if(jbns>N)
		{
			jbns=N;
		}
		SUM=X_DF[ibsl];

		if((ibsl+1)<=jbns)
		{
			for(int kbns=(ibsl+1);kbns<=jbns;kbns++)
			{
				SUM=(SUM-(U_SFF[ibsl][kbns-ibsl+1]*X_DF[kbns]));
			}
		}
		X_DF[ibsl] = SUM;
	}
}

//---------------------------------------------------------------------//