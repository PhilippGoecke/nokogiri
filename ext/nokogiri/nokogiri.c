#include <nokogiri.h>

VALUE mNokogiri ;
VALUE mNokogiriGumbo ;
VALUE mNokogiriHtml4 ;
VALUE mNokogiriHtml4Sax ;
VALUE mNokogiriHtml5 ;
VALUE mNokogiriXml ;
VALUE mNokogiriXmlSax ;
VALUE mNokogiriXmlXpath ;
VALUE mNokogiriXslt ;

VALUE cNokogiriSyntaxError;
VALUE cNokogiriXmlCharacterData;
VALUE cNokogiriXmlElement;
VALUE cNokogiriXmlXpathSyntaxError;

void noko_init_xml_attr(void);
void noko_init_xml_attribute_decl(void);
void noko_init_xml_cdata(void);
void noko_init_xml_comment(void);
void noko_init_xml_document(void);
void noko_init_xml_document_fragment(void);
void noko_init_xml_dtd(void);
void noko_init_xml_element_content(void);
void noko_init_xml_element_decl(void);
void noko_init_xml_encoding_handler(void);
void noko_init_xml_entity_decl(void);
void noko_init_xml_entity_reference(void);
void noko_init_xml_namespace(void);
void noko_init_xml_node(void);
void noko_init_xml_node_set(void);
void noko_init_xml_processing_instruction(void);
void noko_init_xml_reader(void);
void noko_init_xml_relax_ng(void);
void noko_init_xml_sax_parser(void);
void noko_init_xml_sax_parser_context(void);
void noko_init_xml_sax_push_parser(void);
void noko_init_xml_schema(void);
void noko_init_xml_syntax_error(void);
void noko_init_xml_text(void);
void noko_init_xml_xpath_context(void);
void noko_init_xslt_stylesheet(void);
void noko_init_html_document(void);
void noko_init_html_element_description(void);
void noko_init_html_entity_lookup(void);
void noko_init_html_sax_parser_context(void);
void noko_init_html_sax_push_parser(void);
void noko_init_gumbo(void);
void noko_init_test_global_handlers(void);

static ID id_read, id_write, id_external_encoding;


static VALUE
noko_io_read_check(VALUE val)
{
  VALUE *args = (VALUE *)val;
  return rb_funcall(args[0], id_read, 1, args[1]);
}


static VALUE
noko_io_read_failed(VALUE arg, VALUE exc)
{
  return Qundef;
}


int
noko_io_read(void *io, char *c_buffer, int c_buffer_len)
{
  VALUE rb_io = (VALUE)io;
  VALUE rb_read_string, rb_args[2];
  size_t n_bytes_read, safe_len;

  rb_args[0] = rb_io;
  rb_args[1] = INT2NUM(c_buffer_len);

  rb_read_string = rb_rescue(noko_io_read_check, (VALUE)rb_args, noko_io_read_failed, 0);

  if (NIL_P(rb_read_string)) { return 0; }
  if (rb_read_string == Qundef) { return -1; }
  if (TYPE(rb_read_string) != T_STRING) { return -1; }

  n_bytes_read = (size_t)RSTRING_LEN(rb_read_string);
  safe_len = (n_bytes_read > (size_t)c_buffer_len) ? (size_t)c_buffer_len : n_bytes_read;
  memcpy(c_buffer, StringValuePtr(rb_read_string), safe_len);

  return (int)safe_len;
}


static VALUE
noko_io_write_check(VALUE rb_args)
{
  VALUE rb_io = ((VALUE *)rb_args)[0];
  VALUE rb_output = ((VALUE *)rb_args)[1];
  return rb_funcall(rb_io, id_write, 1, rb_output);
}


static VALUE
noko_io_write_failed(VALUE arg, VALUE exc)
{
  return Qundef;
}


int
noko_io_write(void *io, char *c_buffer, int c_buffer_len)
{
  VALUE rb_args[2], rb_n_bytes_written;
  VALUE rb_io = (VALUE)io;
  VALUE rb_enc = rb_funcall(rb_io, id_external_encoding, 0);
  rb_encoding *io_encoding = RB_NIL_P(rb_enc) ? rb_ascii8bit_encoding() : rb_to_encoding(rb_enc);

  rb_args[0] = rb_io;
  rb_args[1] = rb_enc_str_new(c_buffer, (long)c_buffer_len, io_encoding);

  rb_n_bytes_written = rb_rescue(noko_io_write_check, (VALUE)rb_args, noko_io_write_failed, 0);
  if (rb_n_bytes_written == Qundef) { return -1; }

  return NUM2INT(rb_n_bytes_written);
}


int
noko_io_close(void *io)
{
  return 0;
}


void
Init_nokogiri(void)
{
  mNokogiri         = rb_define_module("Nokogiri");
  mNokogiriGumbo    = rb_define_module_under(mNokogiri, "Gumbo");
  mNokogiriHtml4     = rb_define_module_under(mNokogiri, "HTML4");
  mNokogiriHtml4Sax  = rb_define_module_under(mNokogiriHtml4, "SAX");
  mNokogiriHtml5    = rb_define_module_under(mNokogiri, "HTML5");
  mNokogiriXml      = rb_define_module_under(mNokogiri, "XML");
  mNokogiriXmlSax   = rb_define_module_under(mNokogiriXml, "SAX");
  mNokogiriXmlXpath = rb_define_module_under(mNokogiriXml, "XPath");
  mNokogiriXslt     = rb_define_module_under(mNokogiri, "XSLT");

  rb_const_set(mNokogiri, rb_intern("LIBXML_COMPILED_VERSION"), NOKOGIRI_STR_NEW2(LIBXML_DOTTED_VERSION));
  rb_const_set(mNokogiri, rb_intern("LIBXML_LOADED_VERSION"), NOKOGIRI_STR_NEW2(xmlParserVersion));

  rb_const_set(mNokogiri, rb_intern("LIBXSLT_COMPILED_VERSION"), NOKOGIRI_STR_NEW2(LIBXSLT_DOTTED_VERSION));
  rb_const_set(mNokogiri, rb_intern("LIBXSLT_LOADED_VERSION"), NOKOGIRI_STR_NEW2(xsltEngineVersion));

#ifdef NOKOGIRI_PACKAGED_LIBRARIES
  rb_const_set(mNokogiri, rb_intern("PACKAGED_LIBRARIES"), Qtrue);
#  ifdef NOKOGIRI_PRECOMPILED_LIBRARIES
  rb_const_set(mNokogiri, rb_intern("PRECOMPILED_LIBRARIES"), Qtrue);
#  else
  rb_const_set(mNokogiri, rb_intern("PRECOMPILED_LIBRARIES"), Qfalse);
#  endif
  rb_const_set(mNokogiri, rb_intern("LIBXML2_PATCHES"), rb_str_split(NOKOGIRI_STR_NEW2(NOKOGIRI_LIBXML2_PATCHES), " "));
  rb_const_set(mNokogiri, rb_intern("LIBXSLT_PATCHES"), rb_str_split(NOKOGIRI_STR_NEW2(NOKOGIRI_LIBXSLT_PATCHES), " "));
#else
  rb_const_set(mNokogiri, rb_intern("PACKAGED_LIBRARIES"), Qfalse);
  rb_const_set(mNokogiri, rb_intern("PRECOMPILED_LIBRARIES"), Qfalse);
  rb_const_set(mNokogiri, rb_intern("LIBXML2_PATCHES"), Qnil);
  rb_const_set(mNokogiri, rb_intern("LIBXSLT_PATCHES"), Qnil);
#endif

#ifdef LIBXML_ICONV_ENABLED
  rb_const_set(mNokogiri, rb_intern("LIBXML_ICONV_ENABLED"), Qtrue);
#else
  rb_const_set(mNokogiri, rb_intern("LIBXML_ICONV_ENABLED"), Qfalse);
#endif

#ifdef NOKOGIRI_OTHER_LIBRARY_VERSIONS
  rb_const_set(mNokogiri, rb_intern("OTHER_LIBRARY_VERSIONS"), NOKOGIRI_STR_NEW2(NOKOGIRI_OTHER_LIBRARY_VERSIONS));
#endif

#if defined(_WIN32) && !defined(NOKOGIRI_PACKAGED_LIBRARIES)
  /*
   *  We choose *not* to do use Ruby's memory management functions with windows DLLs because of this
   *  issue in libxml 2.9.12:
   *
   *    https://github.com/sparklemotion/nokogiri/issues/2241
   *
   *  If the atexit() issue gets fixed in a future version of libxml2, then we may be able to skip
   *  this config only for the specific libxml2 versions 2.9.12.
   *
   *  Alternatively, now that Ruby has a generational GC, it might be OK to let libxml2 use its
   *  default memory management functions (recall that this config was introduced to reduce memory
   *  bloat and allow Ruby to GC more often); but we should *really* test with production workloads
   *  before making that kind of a potentially-invasive change.
   */
  rb_const_set(mNokogiri, rb_intern("LIBXML_MEMORY_MANAGEMENT"), NOKOGIRI_STR_NEW2("default"));
#else
  rb_const_set(mNokogiri, rb_intern("LIBXML_MEMORY_MANAGEMENT"), NOKOGIRI_STR_NEW2("ruby"));
  xmlMemSetup((xmlFreeFunc)ruby_xfree, (xmlMallocFunc)ruby_xmalloc, (xmlReallocFunc)ruby_xrealloc, ruby_strdup);
#endif

  xmlInitParser();
  exsltRegisterAll();

  if (xsltExtModuleFunctionLookup((const xmlChar *)"date-time", EXSLT_DATE_NAMESPACE)) {
    rb_const_set(mNokogiri, rb_intern("LIBXSLT_DATETIME_ENABLED"), Qtrue);
  } else {
    rb_const_set(mNokogiri, rb_intern("LIBXSLT_DATETIME_ENABLED"), Qfalse);
  }

  cNokogiriSyntaxError = rb_define_class_under(mNokogiri, "SyntaxError", rb_eStandardError);
  noko_init_xml_syntax_error();
  assert(cNokogiriXmlSyntaxError);
  cNokogiriXmlXpathSyntaxError = rb_define_class_under(mNokogiriXmlXpath, "SyntaxError", cNokogiriXmlSyntaxError);

  noko_init_xml_element_content();
  noko_init_xml_encoding_handler();
  noko_init_xml_namespace();
  noko_init_xml_node_set();
  noko_init_xml_reader();
  noko_init_xml_sax_parser();
  noko_init_xml_xpath_context();
  noko_init_xslt_stylesheet();
  noko_init_html_element_description();
  noko_init_html_entity_lookup();

  noko_init_xml_schema();
  noko_init_xml_relax_ng();

  noko_init_xml_sax_parser_context();
  noko_init_html_sax_parser_context();

  noko_init_xml_sax_push_parser();
  noko_init_html_sax_push_parser();

  noko_init_xml_node();
  noko_init_xml_attr();
  noko_init_xml_attribute_decl();
  noko_init_xml_dtd();
  noko_init_xml_element_decl();
  noko_init_xml_entity_decl();
  noko_init_xml_entity_reference();
  noko_init_xml_processing_instruction();
  assert(cNokogiriXmlNode);
  cNokogiriXmlElement = rb_define_class_under(mNokogiriXml, "Element", cNokogiriXmlNode);
  cNokogiriXmlCharacterData = rb_define_class_under(mNokogiriXml, "CharacterData", cNokogiriXmlNode);
  noko_init_xml_comment();
  noko_init_xml_text();
  noko_init_xml_cdata();

  noko_init_xml_document_fragment();
  noko_init_xml_document();
  noko_init_html_document();
  noko_init_gumbo();

  noko_init_test_global_handlers();

  id_read = rb_intern("read");
  id_write = rb_intern("write");
  id_external_encoding = rb_intern("external_encoding");
}
