#pragma once


// XSPF spec: http://www.xspf.org/xspf-v1.html
class xspf : public playlist_loader
{
	public:
		const char *get_extension();
		bool is_our_content_type( const char *p_content_type );
		bool is_associatable();
		bool can_write();

		void open( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort );
		void write( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort );
};
// instance
playlist_loader_factory_t<xspf> xspf_main;
