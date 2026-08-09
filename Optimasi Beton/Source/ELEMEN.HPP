/***********************************************************************/
/* FILE : ELEMEN.HPP                                                   */
/* PENGISIAN ELEMEN BALOK DAN KOLOM                                    */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/

/*******************************/
/* PENGISIAN DATA ELEMEN BALOK */
/*******************************/
void isi_elemen_balok(int no_el_balok)
{
	B=isi(var_b[no_struktur][0+(12*no_el_balok)],sisi_d_B);      // (mm)
   H=isi(var_b[no_struktur][1+(12*no_el_balok)],sisi_d_H);      // (mm)

   DIA1lap=isi(var_b[no_struktur][2+(12*no_el_balok)],DIA_d);   // (mm)
   NL1lap=isi(var_b[no_struktur][3+(12*no_el_balok)],NL_d);
	DIA2lap=isi(var_b[no_struktur][4+(12*no_el_balok)],DIA_d);   // (mm)
   NL2lap=isi(var_b[no_struktur][5+(12*no_el_balok)],NL_d);

   DIA1tum=isi(var_b[no_struktur][6+(12*no_el_balok)],DIA_d);   // (mm)
   NL1tum=isi(var_b[no_struktur][7+(12*no_el_balok)],NL_d);
	DIA2tum=isi(var_b[no_struktur][8+(12*no_el_balok)],DIA_d);   // (mm)
   NL2tum=isi(var_b[no_struktur][9+(12*no_el_balok)],NL_d);

   DIAS=isi(var_b[no_struktur][10+(12*no_el_balok)],DIAS_d);  // (mm)
   Jarak_S=isi(var_b[no_struktur][11+(12*no_el_balok)],JS_d); // (mm)
	L=EL[no_balok[no_el_balok]]; // (m)

   if(fabs(GESER_KI[no_balok[no_el_balok]])
     >fabs(GESER_KA[no_balok[no_el_balok]]))
   {
   	VU = fabs(GESER_KI[no_balok[no_el_balok]]/teta); // (N)
  	}
   else
   {
      VU = fabs(GESER_KA[no_balok[no_el_balok]]/teta); // (N)
   }
}

/**************************/
/* ELEMEN DAERAH LAPANGAN */
/**************************/
void elemen_lapangan(int no_el_balok)
{
  	DIA1=DIA1lap; // Tulangan tarik (mm)
   NL1=NL1lap;
	DIA2=DIA2lap; // Tulangan desak (mm)
   NL2=NL2lap;
   MU=fabs(MLAP[no_balok[no_el_balok]]/(teta)); // momen ditahan (Nm)
}

/*************************/
/* ELEMEN DAERAH TUMPUAN */
/*************************/
void elemen_tumpuan(int no_el_balok)
{
   DIA1=DIA1tum; 	// Tulangan tarik (mm)
   NL1=NL1tum;
	DIA2=DIA2tum; 	// Tulangan desak (mm)
   NL2=NL2tum;

   if(fabs(MTUM_KI[no_balok[no_el_balok]])
     >fabs(MTUM_KA[no_balok[no_el_balok]]))
	{
     	MU=fabs(MTUM_KI[no_balok[no_el_balok]]/teta); // (Nm)
   }
   else
   {
   	MU=fabs(MTUM_KA[no_balok[no_el_balok]]/teta); // (Nm)
   }
}

/*******************************/
/* PENGISIAN DATA ELEMEN KOLOM */
/*******************************/
void isi_elemen_kolom(int no_el_kolom)
{
   sisi   =isi(var_k[no_struktur][0+(5*no_el_kolom)],sisi_d_K);// (mm)
   DIA    =isi(var_k[no_struktur][1+(5*no_el_kolom)],DIA_d);  	// (mm)
	N_DIA  =isi(var_k[no_struktur][2+(5*no_el_kolom)],NL_d);
   DIAS   =isi(var_k[no_struktur][3+(5*no_el_kolom)],DIAS_d); 	// (mm)
   Jarak_S=isi(var_k[no_struktur][4+(5*no_el_kolom)],JS_d);   	// (mm)

   PU  =(-(PK[no_kolom[no_el_kolom]])/(teta));         			// (N)
	MUX =(MKX[no_kolom[no_el_kolom]])/(teta);           			// (Nm)
   MUY =(MKY[no_kolom[no_el_kolom]])/(teta);           			// (Nm)
   VU  =(fabs((GK[no_kolom[no_el_kolom]])/(teta)));  				// (N)
   L=EL[no_kolom[no_el_kolom]];  	 									// (m)
}

/**********************************************/
/* SUBPROGRAM UNTUK MEMANGGIL ISI DATA ELEMEN */
/**********************************************/
float isi(int no_data,float kelompok_data[])
{
	return kelompok_data[no_data];
}

//---------------------------------------------------------------------//
