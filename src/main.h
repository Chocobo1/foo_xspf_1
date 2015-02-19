#pragma once


// XSPF spec: http://www.xspf.org/xspf-v1.html
class xspfMain : public playlist_loader
{
	public:
		const char *get_extension();
		bool is_our_content_type( const char *p_content_type );
		bool is_associatable();
		bool can_write();

		void open( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort );
		void write( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort );
};

// setting
advconfig_checkbox_factory cfg_write_location( "XSPF playlist write location element" ,
	{ 0x40c1b282 , 0xf04a , 0x4a69 , { 0x90 , 0xfe , 0x40 , 0x51 , 0xa0 , 0x3e , 0x1 , 0xa2 } } ,
	advconfig_branch::guid_branch_tagging , 0.0 , true );

// instance
playlist_loader_factory_t<xspfMain> xspf_main_1;
