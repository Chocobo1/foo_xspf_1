#pragma once


void open_helper( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort );
void open_helper_location( const char *p_path , playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , pfc::string8 xml_base[] );
void open_helper_no_location( playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , pfc::list_t<metadb_handle_ptr> *list );
void write_helper( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort , const bool w_location );

void addInfoHelper( const tinyxml2::XMLElement *x_parent , file_info_impl *f , const char *x_name , const char *db_name );
void filterFieldHelper( const tinyxml2::XMLElement *x_parent , pfc::list_t<metadb_handle_ptr> *list , const char *x_name , const char *field );

void pathToUri( const char *in_path , const char *ref_path , pfc::string8 *out );
void uriToPath( const char *in_uri , const char *ref_path , const pfc::string8 base[] , pfc::string8 *out );

void setXmlBase( pfc::string8 base[] , const t_size num , const char *val );
pfc::string8 getXmlBase( const pfc::string8 base[] );

pfc::string8 urlEncodeUtf8( const char *in );
pfc::string8 urlDecodeUtf8( const char *in );
