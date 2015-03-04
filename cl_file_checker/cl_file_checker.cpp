// cl_file_checker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cl/opencl.h>

int _tmain(int argc, _TCHAR* argv[])
{
   if ( argc != 3 )
   {
      printf( "usage:\n\n\tcl_file_checker.exe [*.cl] [kernel_name] \n" );
      return 0;
   }

   // checking the file exits
   FILE * f_source = fopen( argv[ 1 ], "r" );
   if ( !f_source )
   {
      printf( "file not found\n" );
      return 0;
   }

   fseek( f_source, 0, SEEK_END );
   long f_length = ftell( f_source );
   fseek( f_source, 0, SEEK_SET );
   char * p_source_string = ( char * )malloc( f_length );
   fread_s( p_source_string, f_length, f_length, 1, f_source );
   
   cl_int cl_error = CL_SUCCESS;
   cl_platform_id * platform_id = NULL;
   cl_device_id * device_id = NULL;
   cl_context context = NULL;
   cl_program program = NULL;
   cl_kernel kernel = NULL;
   cl_uint cnt = 0;

   do 
   {
      // get platform id
      cl_error = clGetPlatformIDs( 0, NULL, &cnt );
      platform_id = ( cl_platform_id * )malloc( sizeof( cl_platform_id ) * cnt );
      cl_error = clGetPlatformIDs( cnt, platform_id, &cnt );
      if ( FAILED( cl_error ) )
         break;
      // get device id
      cl_error = clGetDeviceIDs( platform_id[ 0 ], CL_DEVICE_TYPE_GPU, 0, NULL, &cnt );
      device_id = ( cl_device_id * )malloc( sizeof( cl_device_id ) * cnt );
      cl_error = clGetDeviceIDs( platform_id[ 0 ], CL_DEVICE_TYPE_GPU, cnt, device_id, &cnt );
      if ( FAILED( cl_error ) )
         break;
      context = clCreateContext( NULL, 1, device_id, NULL, NULL, &cl_error );
      if ( FAILED( cl_error ) )
         break;
      // load file
      const char * source_string_ary [] = {
         p_source_string,
      };
      program = clCreateProgramWithSource(
         context, sizeof( source_string_ary ) / sizeof( source_string_ary[ 0 ] ),
         source_string_ary, NULL, &cl_error );
      if ( FAILED( cl_error ) )
         break;
      cl_error = clBuildProgram( program, 1, device_id, "-cl-fast-relaxed-math", NULL, NULL );
      if ( FAILED( cl_error ) )
      {
         clGetProgramBuildInfo( program, device_id[ 0 ], CL_PROGRAM_BUILD_LOG, 0, NULL, &cnt );
         char * pbi = ( char * )malloc( sizeof( char ) * cnt );
         memset( pbi, 0, sizeof( char ) * cnt );
         clGetProgramBuildInfo( program, device_id[ 0 ], CL_PROGRAM_BUILD_LOG, cnt, pbi, &cnt );
         printf( "build program error with %s\n", pbi );
         break;
      }
      kernel = clCreateKernel( program, argv[ 2 ], &cl_error );
      if ( FAILED( cl_error ) )
         break;
   } while ( FALSE );

   // cleanup
   if ( kernel )
      clReleaseKernel( kernel );
   if ( program )
      clReleaseProgram( program );
   if ( context )
      clReleaseContext( context );
   if ( device_id )
      free( device_id );
   if ( platform_id )
      free( platform_id );
   if ( p_source_string )
      free( p_source_string );
   if ( f_source )
      fclose( f_source );
	return 0;
}

