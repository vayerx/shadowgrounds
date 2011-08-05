
// DO NOT DELETE!!!!!!!


//------------------------------------------------------------------
// Storm3D_Model_Object::CreatePlane
//------------------------------------------------------------------
void Storm3D_Model_Object::CreatePlane(float width,float height,int xsegs,int ysegs,float xtexrep,float ytexrep)
{
	// Delete old faces/vertexes
	DeleteAllFaces();
	DeleteAllVertexes();


	// BETA!!!
/*	Storm3D_Vertex verts[4];
	verts[0]=Storm3D_Vertex(VC3(0,0,0),VC3(0,1,0),VC2(0,0));
	verts[1]=Storm3D_Vertex(VC3(0,0,1),VC3(0,1,0),VC2(0,1));
	verts[2]=Storm3D_Vertex(VC3(1,0,0),VC3(0,1,0),VC2(1,0));
	verts[3]=Storm3D_Vertex(VC3(1,0,1),VC3(0,1,0),VC2(1,1));
	SetVertexes(verts,4);
	WORD ixs[4];
	ixs[0]=0;
	ixs[1]=1;
	ixs[2]=2;
	ixs[3]=3;
	SetFaceStrip(ixs,4);
	return;*/


	// Temp variables
	int x,y;
	int fi=0,vi=0;
	Storm3D_Vertex *vert=new Storm3D_Vertex[(xsegs+1)*(ysegs+1)];
	//Storm3D_Vertex *vert=new Storm3D_Vertex[(xsegs+1)*2];
	//Storm3D_Face *face=new Storm3D_Face[2*xsegs*ysegs];

	// Create vertexes
	int xlen=xsegs+1;
	int ylen=ysegs+1;
	float xadd=width/xsegs;
	float yadd=height/ysegs;
	for (y=0;y<ylen;y++)
	for (x=0;x<xlen;x++)
	{
		// Coordinates
		VC3 pos=
			VC3((-width/2.0f)+(float)x*xadd,0,(-height/2.0f)+(float)y*yadd);

		// Normal vector
		VC3 nvec=VC3(0,1,0);

		// Texturecoordinates
		VC2 tc=
			VC2((1.0f-((float)x/(float)xsegs))*xtexrep,((float)y/(float)ysegs)*ytexrep);
	
		// Add the vertex in to the array
		vert[vi++]=Storm3D_Vertex(pos,nvec,tc);
	}

	/*int xlen=xsegs+1;
	int ylen=ysegs+1;
	float xadd=width/xsegs;
	float yadd=height/ysegs;
	for (y=0;y<ylen/2;y++)	// BETA: parittomalla viim. rivi jää pois (seg oltava pariton)
	for (x=0;x<xlen*2;x++)
	{
		int tx=x/2;
		int ty=y*2+(x%2);

		// Coordinates
		VC3 pos=
			VC3((-width/2.0f)+(float)tx*xadd,0,(-height/2.0f)+(float)ty*yadd);
		
		// Texturecoordinates
		VC2 tc=
			VC2((1.0f-((float)tx/(float)xsegs))*xtexrep,((float)ty/(float)ysegs)*ytexrep);
	
		// Add the vertex in to the array
		vert[vi++]=Storm3D_Vertex(pos,tc);
	}*/

/*
	for (y=0;y<2;y++)
	for (x=0;x<(xsegs+1);x++)
	{
		// Coordinates
		VC3 pos=
			VC3((-width/2.0f)+(float)x*xadd,0,(-height/2.0f)+(float)y*yadd);
		
		// Texturecoordinates
		VC2 tc=
			VC2((1.0f-((float)x/(float)xsegs))*xtexrep,((float)y/(float)ysegs)*ytexrep);
	
		// Add the vertex in to the array
		vert[vi++]=Storm3D_Vertex(pos,tc);
	}*/

/*	// Create faces
	for (y=0;y<ysegs;y++)
	for (x=0;x<xsegs;x++)
	{
		int vind[3];

		// First triangle of quad
		vind[2]=y*(xsegs+1)+x;
		vind[1]=y*(xsegs+1)+x+1;
		vind[0]=(y+1)*(xsegs+1)+x+1;

		// Add the face in to the array
		face[fi++]=Storm3D_Face(vind);

		// Second triangle of quad
		vind[2]=y*(xsegs+1)+x;
		vind[1]=(y+1)*(xsegs+1)+x+1;
		vind[0]=(y+1)*(xsegs+1)+x;

		// Add the face in to the array
		face[fi++]=Storm3D_Face(vind);
	}*/

	// Set faces/vertexes to object
	SetVertexes(vert,vi);
	//SetFaces(face,fi);

	// Create faces
	/*WORD *fs=new WORD[2*(xsegs+2)*ysegs];
	for (y=0;y<ysegs;y++)
	for (x=0;x<(xsegs+2);x++)
	{
		if (x==(xsegs+1))	// Do stripjoin
		{
			// No join at the last row
			if (y<(ysegs-1))
			{
				// Add 2 extra indexes in to the array
				fs[fi++]=(y+1)*(xsegs+1)+x-1;
				fs[fi++]=(y+1)*(xsegs+1);
			}
		}
		else	// Basic stuff
		{
			// Add 2 indexes in to the array
			fs[fi++]=y*(xsegs+1)+x;
			fs[fi++]=(y+1)*(xsegs+1)+x;
		}
	}*/
	
	// Create faces
	WORD *fs=new WORD[2*(xsegs+2)*ysegs];
	int dir=0;
	for (y=0;y<ysegs;y++)
	{
		for (x=0;x<(xsegs+2);x++)
		{
			if (dir==0)
			{
				if (x==(xsegs+1))	// Do stripjoin
				{
					// No join at the last row
					if (y<(ysegs-1))
					{
						// Add 2 extra indexes in to the array
						fs[fi++]=(y+1)*(xsegs+1)+x-1;
						//fs[fi++]=(y+1)*(xsegs+1)+x-1;
					}
				}
				else	// Basic stuff
				{
					// Add 2 indexes in to the array
					fs[fi++]=y*(xsegs+1)+x;
					fs[fi++]=(y+1)*(xsegs+1)+x;
				}
			}
			else
			{
				if (x==(xsegs+1))	// Do stripjoin
				{
					// No join at the last row
					if (y<(ysegs-1))
					{
						// Add 2 extra indexes in to the array
						fs[fi++]=(y+1)*(xsegs+1);
						//fs[fi++]=(y+1)*(xsegs+1);
					}
				}
				else	// Basic stuff
				{
					// Add 2 indexes in to the array
					fs[fi++]=y*(xsegs+1)+(xsegs-x);
					fs[fi++]=(y+1)*(xsegs+1)+(xsegs-x);
				}
			}
		}

		// Change the direction (in each row)
		if (dir==0) dir=1; else dir=0;
	}

	
	// Create faces
	/*WORD *fs=new WORD[2*(xsegs+2)*ysegs];
	int dir=0;
	for (y=0;y<ysegs;y++)
	{
		for (x=0;x<(xsegs+2);x++)
		{
			if (dir==0)
			{
				if (x==(xsegs+1))	// Do stripjoin
				{
					// No join at the last row
					if (y<(ysegs-1))
					{
						// Add 2 extra indexes in to the array
						fs[fi++]=y*(xsegs+1)+1+(x-1)*2;
					}
				}
				else	// Basic stuff (super cache usage!)
				{
					// Add 2 indexes in to the array
					fs[fi++]=y*(xsegs+1)+x*2;
					fs[fi++]=y*(xsegs+1)+1+x*2;
				}
			}
			else
			{
				if (x==(xsegs+1))	// Do stripjoin
				{
					// No join at the last row
					if (y<(ysegs-1))
					{
						// Add 2 extra indexes in to the array
						fs[fi++]=(xsegs+1)*(3+y)-2-(x-1)*2;
					}
				}
				else	// Basic stuff (sucks!;)
				{
					// Add 2 indexes in to the array
					fs[fi++]=(xsegs+1)*(1+y)-1-x*2;
					fs[fi++]=(xsegs+1)*(3+y)-2-x*2;
				}
			}
		}

		// Change the direction (in each row)
		if (dir==0) dir=1; else dir=0;
	}*/
	

	/*WORD *fs=new WORD[2*(xsegs+1)];
	for (x=0;x<(xsegs+1);x++)
	{
		// Add 2 indexes in to the array
		fs[fi++]=(xsegs+1)+x;
		fs[fi++]=x;
	}*/

	/*for (x=0;x<10000;x++)
	{
		fs[fi++]=rand()%(vertex_amount/1000);
	}*/

	// Test array size!
	/*if (fi>=65535)
	{
		DeleteAllFaces();
		DeleteAllVertexes();
		delete[] vert;
		delete[] fs;
		return;
	}

	// BETA!: Test array
	for (int i=0;i<fi;i++) if (fs[i]>=vertex_amount)
	{
		DeleteAllFaces();
		DeleteAllVertexes();
		delete[] vert;
		delete[] fs;
		return;
	}*/

	// Set facestrip to object
	SetFaceStrip(fs,fi);

	// Delete temp arrays
	delete[] vert;
	//delete[] face;
	delete[] fs;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::CreateSphere
//------------------------------------------------------------------
void Storm3D_Model_Object::CreateSphere(float radius,int rsegs,int hsegs,float rtexrep,float htexrep)
{
	// Delete old faces/vertexes
	DeleteAllFaces();
	DeleteAllVertexes();

	// Temp variables
	int x,y;
	int fi=0,vi=0;

	// BETA!!!
	/*BYTE tab[100][200];
	FILE *f;
	f=fopen("st3d.bm","rb");
	fread(tab,200,100,f);
	fclose(f);*/

	// Create vertexes
	Storm3D_Vertex *vert=new Storm3D_Vertex[(rsegs+1)*(hsegs+1)];
	int xlen=rsegs+1;
	int ylen=hsegs+1;
	float xadd=(PI*2.0f)/((float)rsegs);	// Precalculate
	for (y=0;y<ylen;y++)
	{	
		// Precalculate
		float yc=cosf((((float)y)/(float)hsegs)*PI)*radius*0.5f;
		float x_mul=sinf((((float)y)/(float)hsegs)*PI)*radius*0.5f;
		
		for (x=0;x<xlen;x++)
		{
			// Coordinates
			VC3 pos=
				VC3(sinf((float)x*xadd)*x_mul,yc,cosf((float)x*xadd)*x_mul);

			// Normal vector
			VC3 nvec=pos/(radius*0.5f);

			// BETA!!
			/*nvec=nvec+cosf(x)*0.2f;
			nvec.Normalize();
			pos=pos*(1+sinf(x)*0.01f);*/
			
			/*int xt=x;if (xt>198) xt=198;
			int yt=y;if (yt>99) yt=99;
			float bn=((tab[yt][xt+1])-(tab[yt][xt]))*0.03;
			nvec=nvec+bn*0.2f;
			nvec.Normalize();*/
			
			//pos=pos*(1.0f+tab[xt][yt]*0.01f);

			// Texturecoordinates
			VC2 tc=
				VC2((1.0f-((float)x/(float)rsegs))*rtexrep,((float)y/(float)hsegs)*htexrep);
	
			// Add the vertex in to the array
			vert[vi++]=Storm3D_Vertex(pos,nvec,tc);
		}
	}

	// Set faces/vertexes to object
	SetVertexes(vert,vi);

	// Create faces
	/*WORD *fs=new WORD[2*(rsegs+2)*hsegs];
	for (y=0;y<hsegs;y++)
	for (x=0;x<(rsegs+2);x++)
	{
		if (x==(rsegs+1))	// Do stripjoin
		{
			// No join at the last row
			if (y<(hsegs-1))
			{
				// Add 2 extra indexes in to the array
				fs[fi++]=(y+1)*(rsegs+1)+x-1;
				fs[fi++]=(y+1)*(rsegs+1);
			}
		}
		else	// Basic stuff
		{
			// Add 2 indexes in to the array
			fs[fi++]=y*(rsegs+1)+x;
			fs[fi++]=(y+1)*(rsegs+1)+x;
		}
	}*/

	// Create faces
	int xsegs=rsegs;
	int ysegs=hsegs;
	WORD *fs=new WORD[2*(xsegs+2)*ysegs];
	int dir=0;
	for (y=0;y<ysegs;y++)
	{
		for (x=0;x<(xsegs+2);x++)
		{
			if (dir==0)
			{
				if (x==(xsegs+1))	// Do stripjoin
				{
					// No join at the last row
					if (y<(ysegs-1))
					{
						// Add 2 extra indexes in to the array
						fs[fi++]=(y+1)*(xsegs+1)+x-1;
						//fs[fi++]=(y+1)*(xsegs+1)+x-1;
					}
				}
				else	// Basic stuff
				{
					// Add 2 indexes in to the array
					fs[fi++]=y*(xsegs+1)+x;
					fs[fi++]=(y+1)*(xsegs+1)+x;
				}
			}
			else
			{
				if (x==(xsegs+1))	// Do stripjoin
				{
					// No join at the last row
					if (y<(ysegs-1))
					{
						// Add 2 extra indexes in to the array
						fs[fi++]=(y+1)*(xsegs+1);
						//fs[fi++]=(y+1)*(xsegs+1);
					}
				}
				else	// Basic stuff
				{
					// Add 2 indexes in to the array
					fs[fi++]=y*(xsegs+1)+(xsegs-x);
					fs[fi++]=(y+1)*(xsegs+1)+(xsegs-x);
				}
			}
		}

		// Change the direction (in each row)
		if (dir==0) dir=1; else dir=0;
	}
	
	// Set facestrip to object
	SetFaceStrip(fs,fi);

	// Delete temp arrays
	delete[] vert;
	delete[] fs;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::CreateBox
//------------------------------------------------------------------
void Storm3D_Model_Object::CreateBox(float width,float length,float height,int xsegs,int ysegs,float xtexrep,float ytexrep)
{
	// Delete old faces/vertexes
	DeleteAllFaces();
	DeleteAllVertexes();

	// Temp variables
	int x,y,side;
	int fi=0,vi=0;

	// Create vertexes
	Storm3D_Vertex *vert=new Storm3D_Vertex[(xsegs+1)*(ysegs+1)*6];
	float xadd,yadd;
	int xlen=xsegs+1;
	int ylen=ysegs+1;
	for (side=0;side<6;side++)
	for (y=0;y<ylen;y++)
	for (x=0;x<xlen;x++)
	{
		// Coordinates and normal vector
		VC3 pos;		
		VC3 nvec;

		switch (side)
		{
			case 0:	// Y+ (TOP)
				xadd=width/xsegs;
				yadd=height/ysegs;
				nvec=VC3(0,1,0);
				pos=VC3((-width/2.0f)+(float)x*xadd,
					length/2,(-height/2.0f)+(float)y*yadd);
				break;

			case 1:	// Y- (BOTTOM)
				xadd=width/xsegs;
				yadd=height/ysegs;
				nvec=VC3(0,-1,0);
				pos=VC3((width/2.0f)-(float)x*xadd,
					-length/2,(-height/2.0f)+(float)y*yadd);
				break;

			case 2:	// X+ (LEFT)
				xadd=height/xsegs;
				yadd=length/ysegs;
				nvec=VC3(1,0,0);
				pos=VC3(width/2,(length/2.0f)-(float)y*yadd,
					(height/2.0f)-(float)x*xadd);
				break;

			case 3:	// X- (RIGHT)
				xadd=height/xsegs;
				yadd=length/ysegs;
				nvec=VC3(-1,0,0);
				pos=VC3(-width/2,(length/2.0f)-(float)y*yadd,
					(-height/2.0f)+(float)x*xadd);
				break;

			case 4:	// Z+ (FRONT)
				xadd=width/xsegs;
				yadd=length/ysegs;
				nvec=VC3(0,0,1);
				pos=VC3((-width/2.0f)+(float)x*xadd,
					(length/2.0f)-(float)y*yadd,height/2);
				break;

			case 5:	// Z- (BOTTOM)
				xadd=width/xsegs;
				yadd=length/ysegs;
				nvec=VC3(0,0,-1);
				pos=VC3((width/2.0f)-(float)x*xadd,
					(length/2.0f)-(float)y*yadd,-height/2);
				break;
		}
		
		// Texturecoordinates
		VC2 tc=
			VC2((1.0f-((float)x/(float)xsegs))*xtexrep,((float)y/(float)ysegs)*ytexrep);
	
		// Add the vertex in to the array
		vert[vi++]=Storm3D_Vertex(pos,nvec,tc);
	}

	// Set faces/vertexes to object
	SetVertexes(vert,vi);

	// Create faces
	/*WORD *fs=new WORD[2*(xsegs+2)*ysegs*6];
	int sx=(xsegs+1)*(ysegs+1);
	for (side=0;side<6;side++)
	{
		for (y=0;y<ysegs;y++)
		for (x=0;x<(xsegs+2);x++)
		{
			if (x==(xsegs+1))	// Do stripjoin
			{
				// No join at the last row
				if (y<(ysegs-1))
				{
					// Add 2 extra indexes in to the array
					fs[fi++]=sx*side+(y+1)*(xsegs+1)+x-1;
					fs[fi++]=sx*side+(y+1)*(xsegs+1);
				}
			}
			else	// Basic stuff
			{
				// Add 2 indexes in to the array
				fs[fi++]=sx*side+y*(xsegs+1)+x;
				fs[fi++]=sx*side+(y+1)*(xsegs+1)+x;
			}
		}

		// Do side stripjoin (not at the last side)
		if (side<5)
		{
			fs[fi++]=sx*(side+1)-1;
			fs[fi++]=sx*(side+1);
		}
	}*/
	
	// Create faces
	WORD *fs=new WORD[2*(xsegs+2)*ysegs*6];
	int sx=(xsegs+1)*(ysegs+1);
	for (side=0;side<6;side++)
	{
		int dir=0;
		for (y=0;y<ysegs;y++)
		{
			for (x=0;x<(xsegs+2);x++)
			{
				if (dir==0)
				{
					if (x==(xsegs+1))	// Do stripjoin
					{
						// No join at the last row
						if (y<(ysegs-1))
						{
							// Add 2 extra indexes in to the array
							fs[fi++]=sx*side+(y+1)*(xsegs+1)+x-1;
							//fs[fi++]=(y+1)*(xsegs+1)+x-1;
						}
					}
					else	// Basic stuff
					{
						// Add 2 indexes in to the array
						fs[fi++]=sx*side+y*(xsegs+1)+x;
						fs[fi++]=sx*side+(y+1)*(xsegs+1)+x;
					}
				}
				else
				{
					if (x==(xsegs+1))	// Do stripjoin
					{
						// No join at the last row
						if (y<(ysegs-1))
						{
							// Add 2 extra indexes in to the array
							fs[fi++]=sx*side+(y+1)*(xsegs+1);
							//fs[fi++]=(y+1)*(xsegs+1);
						}
					}
					else	// Basic stuff
					{
						// Add 2 indexes in to the array
						fs[fi++]=sx*side+y*(xsegs+1)+(xsegs-x);
						fs[fi++]=sx*side+(y+1)*(xsegs+1)+(xsegs-x);
					}
				}
			}

			// Change the direction (in each row)
			if (dir==0) dir=1; else dir=0;
		}

		// Do side stripjoin (not at the last side)
		if (side<5)
		{
			if ((ysegs%2)==0) fs[fi++]=sx*(side+1)-1; // If strip ends at wrong culling order: do an extra face
			fs[fi++]=sx*(side+1)-1;
			fs[fi++]=sx*(side+1);
		}
	}
	
	// Set facestrip to object
	SetFaceStrip(fs,fi);

	// Delete temp arrays
	delete[] vert;
	delete[] fs;
}



