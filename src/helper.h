#pragma once


void open_helper( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort );
void write_helper( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort , const bool w_location );

void locationToUri( const char *in_path , const char *ref_path , pfc::string8 *out );
void uriToLocation( const char *in_uri , const char *ref_path , const pfc::string8 base[] , pfc::string8 *out );

void setXmlBase( pfc::string8 base[] , const t_size num , const char *val );
pfc::string8 getXmlBase( const pfc::string8 base[] );

pfc::string8 urlEncodeUtf8( const char *in );
pfc::string8 urlDecodeUtf8( const char *in );
