#include "lwo.h"
#include "lwReader.h"
#include "lwo2mesh.h"
#include <io.h>
#include <time.h>

using namespace Ogre;
using namespace std;

ostream& nl(ostream& os)
{
	return os << '\n';
}

int nobjects = 0, nlayers = 0, nsurfs = 0, nenvs = 0, nclips = 0, npoints = 0, npolygons = 0;

/*
======================================================================
print_vmaps1()

  Print vmap values for a layer, looping through the vmaps.
  
	for each vmap
	print vmap statistics
	for each mapped point
	print point index and position
	if vmad, print polygon index and vertex number
	print vmap values
====================================================================== */

static void print_vmaps1( FILE *fp, lwLayer *layer )
{
	lwPoint *pt;
	lwVMap *vmap;
	char *tag;
	unsigned int i, j, k, n;
	
	fprintf( fp, "\n\nVertex Maps (%d)\n\n", layer->vmaps.size() );
	
	for ( i = 0; i < layer->vmaps.size(); i++ )
	{
		vmap = layer->vmaps[i];

		tag = ( char * ) &vmap->type;
		
		fprintf( fp, "%c%c%c%c \"%s\"  dim %d  nverts %d  vmad (%s)\n",
			tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ],
			vmap->name,
			vmap->dim,
			vmap->nverts,
			vmap->perpoly ? "yes" : "no" );
		
		printf( "%c%c%c%c \"%s\"  dim %d  nverts %d  vmad (%s)\n",
			tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ],
			vmap->name,
			vmap->dim,
			vmap->nverts,
			vmap->perpoly ? "yes" : "no" );
		
		for ( j = 0; j < vmap->nverts; j++ ) {
			/* point index */
			fprintf( fp, "  point %d ", vmap->vindex[ j ] );
			
			/* if vmad */
			if ( vmap->perpoly ) {
				lwPolygon *pol;
				
				/* polygon index */
				k = vmap->pindex[ j ];
				fprintf( fp, " poly %d ", k );
				
				/* vertex index */
				pol = layer->polygons[ k ];
				for ( n = 0; n < pol->vertices.size(); n++ )
					if ( pol->vertices[ n ]->index == vmap->vindex[ j ] ) break;
					fprintf( fp, " vert %d ", n );
			}
			
			/* point coords */
			pt = layer->points[ vmap->vindex[ j ]];
			fprintf( fp, " (%g, %g, %g) ", pt->x, pt->y, pt->z );
			
			/* vmap values */
			for ( k = 0; k < vmap->dim; k++ )
				fprintf( fp, " %g", vmap->val[ j ][ k ] );
			
			/* done with this point */
			fprintf( fp, "\n" );
		}
		/* done with this vmap */
		fprintf( fp, "\n" );
	}
	/* done with this layer */
	fprintf( fp, "\n\n" );
}


/*
======================================================================
print_vmaps2()

  Print vmap values for a layer, looping through the points.
  
	for each point
	print point index, position, number of vmaps, polygon indexes
	for each vmap on the point
	print vmap name, type and values
====================================================================== */

static void print_vmaps2( FILE *fp, lwLayer *layer )
{
	lwPoint *pt;
	lwVMap *vmap;
	char *tag;
	unsigned int i, j, k, n;
	
	fprintf( fp, "\n\nPoints (%d)\n\n", layer->points.size() );
	
	for ( i = 0; i < layer->points.size(); i++ ) {
		pt = layer->points[ i ];
		
		/* point index and position */
		fprintf( fp, "%d (%g, %g, %g)", i, pt->x, pt->y, pt->z );
		
		/* number of vmaps and polygons */
		fprintf( fp, "  nvmaps %d  npolygons %d", pt->vmaps.size(), pt->polygons.size() );
		
		/* polygon indexes */
		fprintf( fp, " [" );
		for ( j = 0; j < pt->polygons.size(); j++ )
			fprintf( fp, " %d", pt->polygons[ j ] );
		fprintf( fp, "]\n" );
		
		/* vmaps for this point */
		for ( j = 0; j < pt->vmaps.size(); j++ ) {
			vmap = pt->vmaps[ j ].vmap;
			n = pt->vmaps[ j ].index;
			
			tag = ( char * ) &vmap->type;
			
			fprintf( fp, "  %c%c%c%c \"%s\" vmad (%s)",
				tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ], vmap->name,
				vmap->perpoly ? "yes" : "no" );
			
			/* vmap values */
			for ( k = 0; k < vmap->dim; k++ )
				fprintf( fp, " %g", vmap->val[ n ][ k ] );
			
			/* done with this vmap */
			fprintf( fp, "\n" );
		}
		/* done with this point */
		fprintf( fp, "\n" );
	}
	/* done with this layer */
	fprintf( fp, "\n\n" );
}


/*
======================================================================
print_vmaps3()

  Print vmap values for a layer, looping through the polygons.
  
	for each polygon
	print polygon index, number of points
	for each vertex
	print point index, position, number of vmaps
	for each vmap on the point
	print vmap name, type and values
====================================================================== */

static void print_vmaps3( FILE *fp, lwLayer *layer )
{
	lwPoint *pt;
	lwPolygon *pol;
	lwVMap *vmap;
	char *tag;
	unsigned int i, j, k, m, n;
	
	fprintf( fp, "\n\nPolygons (%d)\n\n", layer->polygons.size() );
	
	for ( i = 0; i < layer->polygons.size(); i++ ) {
		pol = layer->polygons[ i ];
		
		/* polygon index, type, number of vertices */
		tag = ( char * ) &pol->type;
		fprintf( fp, "%d %c%c%c%c  nverts %d\n", i,
			tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ], pol->vertices.size() );
		
		for ( k = 0; k < pol->vertices.size(); k++ ) {
			/* point index, position, number of vmads and vmaps */
			n = pol->vertices[ k ]->index;
			pt = layer->points[ n ];
			fprintf( fp, "%d (%g, %g, %g)  nvmads %d  nvmaps %d\n", n,
				pt->x, pt->y, pt->z,
				pol->vertices[ k ]->vmaps.size(), pt->vmaps.size() - pol->vertices[ k ]->vmaps.size() );
			
			/* vmads for this vertex */
			for ( j = 0; j < pol->vertices[ k ]->vmaps.size(); j++ ) {
				vmap = pol->vertices[ k ]->vmaps[ j ].vmap;
				n = pol->vertices[ k ]->vmaps[ j ].index;
				
				tag = ( char * ) &vmap->type;
				fprintf( fp, "  %c%c%c%c vmad \"%s\"",
					tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ], vmap->name );
				
				/* vmap values */
				for ( m = 0; m < vmap->dim; m++ )
					fprintf( fp, " %g", vmap->val[ m ][ n ] );
				
				/* done with this vmad */
				fprintf( fp, "\n" );
			}
			
			/* vmaps for this vertex */
			for ( j = 0; j < pt->vmaps.size(); j++ ) {
				vmap = pt->vmaps[ j ].vmap;
				if ( vmap->perpoly ) continue;
				n = pt->vmaps[ j ].index;
				
				tag = ( char * ) &vmap->type;
				fprintf( fp, "  %c%c%c%c vmap \"%s\"",
					tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ], vmap->name );
				
				/* vmap values */
				for ( m = 0; m < vmap->dim; m++ )
					fprintf( fp, " %g", vmap->val[ m ][ n ] );
				
				/* done with this vmap */
				fprintf( fp, "\n" );
			}
			
			/* done with this vertex */
			if ( pt->vmaps.size() )
				fprintf( fp, "\n" );
		}
		/* done with this polygon */
		fprintf( fp, "\n" );
	}
	/* done with this layer */
	fprintf( fp, "\n\n" );
}


/*
======================================================================
print_vmaps()

  Print vmap values for a layer.
  
	Calls print_vmaps1(), print_vmaps2() and print_vmaps3().
====================================================================== */

void print_vmaps( lwObject *obj )
{
	FILE *fp[ 3 ];
	char buf[ 64 ];
	lwLayer *layer;
	unsigned int i, j;
	
	for ( i = 0; i < 3; i++ ) {
		sprintf( buf, "vmapout%d.txt", i + 1 );
		fp[ i ] = fopen( buf, "w" );
		if ( !fp[ i ] ) {
			for ( j = i - 1; j >= 0; j-- )
				fclose( fp[ j ] );
			return;
		}
	}
	
	for ( i = 0; i < obj->layers.size(); i++ )
	{
		layer = obj->layers[i];

		fprintf( fp[ 0 ], "------------------------\nLayer %d\n", i );
		print_vmaps1( fp[ 0 ], layer );
		
		fprintf( fp[ 1 ], "------------------------\nLayer %d\n", i );
		print_vmaps2( fp[ 1 ], layer );
		
		fprintf( fp[ 2 ], "------------------------\nLayer %d\n", i );
		print_vmaps3( fp[ 2 ], layer );
	}
	
	for ( i = 0; i < 3; i++ )
		fclose( fp[ i ] );
}

int make_filename( char *spec, char *name, char *fullname )
{
	char
		drive[ _MAX_DRIVE ],
		dir[ _MAX_DIR ],
		node[ _MAX_FNAME ],
		ext[ _MAX_EXT ];
	
	_splitpath( spec, drive, dir, node, ext );
	_makepath( fullname, drive, dir, name, NULL );
	return 1;
}

int make_destname( char *spec, char *name, char *fullname )
{
	char
		drive[ _MAX_DRIVE ],
		dir[ _MAX_DIR ],
		node[ _MAX_FNAME ],
		ext[ _MAX_EXT ],
		dnode[ _MAX_FNAME ];
	
	_splitpath( name, drive, dir, dnode, ext );
	_splitpath( spec, drive, dir, node, ext );
	_makepath( fullname, drive, dir, dnode, ".mesh" );
	return 1;
}

int make_filespec( char *spec, char *subdir, char *fullname )
{
	char
		name[ _MAX_FNAME ],
		drive[ _MAX_DRIVE ],
		dir[ _MAX_DIR ],
		node[ _MAX_FNAME ],
		ext[ _MAX_EXT ];
	
	_splitpath( spec, drive, dir, node, ext );
	_makepath( name, drive, dir, subdir, NULL );
	_makepath( fullname, NULL, name, node, ext );
	return 1;
}

void help( char *filename )
{
	cout << "lwo2mesh v0.87 (2003.07.20) by Dennis Verbeek." << nl
		<< "Converts a Lightwave object to an Ogre mesh." << nl
		<< "Please send any feedback to: dennis.verbeek@chello.nl" << nl << nl
		<< "Usage: " << filename << " [options] source [dest]" << nl
		<< "options:" << nl
		<< "-g do not use shared geometry" << nl
		<< "-i info on .lwo only, no conversion to mesh" << nl
		<< "   -v dump vertex maps" << nl
		<< "-l save layers separately" << nl
		<< "-m do not export materials" << nl
		<< "-s do not export skeleton" << endl;
	exit(0);
}

void info( lwObject *object, char *filename, bool flags[] )
{
	unsigned int points = 0;
	unsigned int polygons = 0;

	nlayers += object->layers.size();
	nsurfs += object->surfaces.size();
	nclips += object->clips.size();
	nenvs += object->envelopes.size();

	cout << "File: " << filename << nl
		<< setw(8) << object->layers.size() << " layers" << nl
		<< setw(8) << object->surfaces.size() << " surfaces" << nl
		<< setw(8) << object->clips.size() << " clips" << nl
		<< setw(8) << object->envelopes.size() << " envelopes" << endl;
	
	if (object->layers.size() > 1)
	{
		for( unsigned int i = 0; i < object->layers.size(); i++ )
		{
			points += object->layers[i]->points.size();
			cout << setw(8) << object->layers[i]->points.size() << " points (layer " << i;
			
			if (object->layers[i]->name)
				cout << ", " << object->layers[i]->name << ")" << endl;
			else
				cout << ")" << endl;
			
			polygons += object->layers[i]->polygons.size();
			cout << setw(8) << object->layers[i]->polygons.size() << " polygons (layer " << i;
			
			if (object->layers[i]->name)				
				cout << ", " << object->layers[i]->name << ")" << endl;
			else
				cout << ")" << endl;
		}
	}
	else
	{
		points += object->layers[0]->points.size();
		polygons += object->layers[0]->polygons.size();
	}
	
	cout << setw(8) << points << " points (total)" << nl
		<< setw(8) << polygons << " polygons (total)" << nl << endl;
	
	nobjects++;
	npoints += points;
	npolygons += polygons;
	
	if (flags[PrintVMaps])
		print_vmaps( object );
}

int readFiles( char *source, char *dest, bool flags[] )
{
	long h, err;
	struct _finddata_t data;
	char *filename, *prevname, *destname;
	unsigned int failID;
	int failpos;
	lwReader reader;
	Lwo2MeshWriter lwo2mesh;
	
	filename = (char *)malloc(3 * 512);
	if ( !filename ) return 0;
	prevname = filename + 512;
	destname = filename + 1024;
	
	err = h = _findfirst( source, &data );
	if ( err == -1 )
	{
		printf( "No files found: '%s'\n", source );
		return 0;
	}
	
	while ( err != -1 )
	{
		if (( data.attrib & _A_SUBDIR ) && data.name[ 0 ] != '.' )
		{
			make_filespec( source, data.name, filename );
			readFiles( filename, dest, flags );
		}
		if ( !( data.attrib & _A_SUBDIR ))
		{
			make_filename( source, data.name, filename );

			if ( !strcmp( filename, prevname )) break;
			strcpy( prevname, filename );
			failID = failpos = 0;

			lwObject *object = reader.readObjectFromFile( filename, flags );
			if ( object )
			{
				if (flags[InfoOnly])
					info(object, filename, flags);
				else
				{
					make_destname( dest, data.name, destname );
					lwo2mesh.writeLwo2Mesh(object, destname, flags);
				}
				delete object;
			}
		}
		err = _findnext( h, &data );
	}
	
	_findclose( h );
	free (filename);
	return 1;
}

void main( int argc, char *argv[] )
{
	int i;
	char *source = 0;
	char *dest = 0;
	bool flags[NUMFLAGS] =
	{
		false, // lightwave info
		false, // dump vmaps
		true,  // shared geometry
		false, // layers
		true,  // materials
		true,  // skeleton
		true,  // has normals
		true,  // new submesh
		true   // linear copy
	};

	if ( argc < 2 ) help(argv[0]);

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-' || argv[i][0] == '/')
		{
			switch (argv[i][1])
			{
			case 'g':
			case 'G':
				flags[UseSharedGeometry] = false;
				break;
			case 'i':
			case 'I':
				flags[InfoOnly] = true;
				break;
			case 'l':
			case 'L':
				flags[UseSeparateLayers] = true;
				break;
			case 'm':
			case 'M':
				flags[ExportMaterials] = false;
				break;
			case 's':
			case 'S':
				flags[ExportSkeleton] = false;
				break;
			case 'v':
			case 'V':
				flags[PrintVMaps] = true;
				break;
			default:
				help(argv[0]);
			}
		}
		else
		{
			if (!source) source = argv[i];
			else
				if (!dest) dest = argv[i];
		}
	}

	if ( !source ) help(argv[0]);

	if (!dest) dest = "\0";

	float t1, t2;
	
	t1 = ( float ) clock() / CLOCKS_PER_SEC;

	if ( strchr(source, '*') )
		readFiles( source, dest, flags );
	else
	{
		lwReader reader;
		lwObject *object = reader.readObjectFromFile( source, flags );
		if ( object )
		{
			if (flags[InfoOnly])
				info(object, source, flags);
			else
			{
				char *destname = (char *)malloc(512);
				if ( !destname ) return;

				if (strlen(dest))
					make_destname(dest, dest, destname);
				else
					make_destname(dest, source, destname);

				Lwo2MeshWriter lwo2mesh;
				lwo2mesh.writeLwo2Mesh(object, destname, flags);
				free(destname);
			}
			delete object;
		}
	}

	t2 = ( float ) clock() / CLOCKS_PER_SEC - t1;
	
	if (flags[InfoOnly])
	{
		cout << "Total:" << nl
			<< setw(8) << nobjects << " objects" << nl
			<< setw(8) << nlayers << " layers" << nl
			<< setw(8) << nsurfs << " surfaces" << nl
			<< setw(8) << nclips << " clips" << nl
			<< setw(8) << nenvs << " envelopes" << nl
			<< setw(8) << npoints << " points" << nl
			<< setw(8) << npolygons << " polygons" << nl
			<< setw(8) << t2 << " seconds processing time." << endl;
	}
}