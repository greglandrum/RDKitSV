//
// file MolDraw2DCairo.cc
// Greg Landrum
// 28 Dec 2014
//
// derived from Dave Cosgrove's MolDraw2DQT
//

#include "MolDraw2DCairo.H"
#include <cairo.h>

namespace RDKit {

  namespace {
  }
  
  void MolDraw2DCairo::initDrawing() {
    cairo_select_font_face(d_cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    
  }

  // ****************************************************************************
  void MolDraw2DCairo::finishDrawing() {
  }

  // ****************************************************************************
  void MolDraw2DCairo::setColour( const DrawColour &col ) {
    MolDraw2D::setColour( col );
    cairo_set_source_rgb(d_cr, col.get<0>(), col.get<1>(), col.get<2>());
  }

  // ****************************************************************************
  void MolDraw2DCairo::drawLine( const std::pair<float,float> &cds1 ,
                               const std::pair<float,float> &cds2 ) {

    std::pair<float,float> c1 = getDrawCoords( cds1 );
    std::pair<float,float> c2 = getDrawCoords( cds2 );

    unsigned int width=2;
    std::string dashString="";

    cairo_set_line_width(d_cr,width);
    cairo_set_dash(d_cr,0,0,0);
    cairo_move_to(d_cr,c1.first,c1.second);
    cairo_line_to(d_cr,c2.first,c2.second);
    cairo_stroke(d_cr);
  }

  // ****************************************************************************
  // draw the char, with the bottom left hand corner at cds
  void MolDraw2DCairo::drawChar( char c , const std::pair<float,float> &cds ) {
    char txt[2];
    txt[0]=c;
    txt[1]=0;

    cairo_text_extents_t extents;
    cairo_text_extents(d_cr,txt,&extents);
    double twidth=extents.width,theight=extents.height;

    unsigned int fontSz=scale()*fontSize();
    std::pair<float,float> c1 = cds ;// getDrawCoords( cds );

    cairo_move_to(d_cr,c1.first,c1.second+theight);
    cairo_show_text(d_cr,txt);
    cairo_stroke(d_cr);
    
  }

  // ****************************************************************************
  void MolDraw2DCairo::drawTriangle( const std::pair<float , float> &cds1 ,
                                   const std::pair<float , float> &cds2 ,
                                   const std::pair<float, float> &cds3 ) {
    std::pair<float,float> c1 = getDrawCoords( cds1 );
    std::pair<float,float> c2 = getDrawCoords( cds2 );
    std::pair<float,float> c3 = getDrawCoords( cds3 );

    unsigned int width=1;
    cairo_set_line_width(d_cr,width);
    cairo_set_dash(d_cr,0,0,0);
    cairo_move_to(d_cr,c1.first,c1.second);
    cairo_line_to(d_cr,c2.first,c2.second);
    cairo_line_to(d_cr,c3.first,c3.second);
    cairo_close_path(d_cr);
    cairo_fill_preserve(d_cr);
    cairo_stroke(d_cr);

  }

  // ****************************************************************************
  void MolDraw2DCairo::clearDrawing() {
    
    cairo_set_source_rgb (d_cr, 0.9, 0.9, 0.9);
    cairo_rectangle(d_cr,0,0,width(),height());
    cairo_fill(d_cr);
  }


  
  // ****************************************************************************
  void MolDraw2DCairo::setFontSize( float new_size ) {
    MolDraw2D::setFontSize( new_size );
    float font_size_in_points = fontSize() * scale();
    cairo_set_font_size (d_cr, font_size_in_points);
  }

  // ****************************************************************************
  // using the current scale, work out the size of the label in molecule coordinates
  void MolDraw2DCairo::getStringSize( const std::string &label , float &label_width ,
                                    float &label_height ) const {
    label_width = 0.0;
    label_height = 0.0;

    int draw_mode = 0; // 0 for normal, 1 for superscript, 2 for subscript

    bool had_a_super = false;

    char txt[2];
    txt[1]=0;
    for( int i = 0 , is = label.length() ; i < is ; ++i ) {

      // setStringDrawMode moves i along to the end of any <sub> or <sup>
      // markup
      if( '<' == label[i] && setStringDrawMode( label , draw_mode , i ) ) {
        continue;
      }


      txt[0]=label[i];
      cairo_text_extents_t extents;
      cairo_text_extents(d_cr,txt,&extents);
      double twidth=extents.x_advance,theight=extents.height;

      label_height = theight/scale();
      float char_width = twidth/scale();

      if( 2 == draw_mode ) {
        char_width *= 0.75;
      } else if( 1 == draw_mode ) {
        char_width *= 0.75;
        had_a_super = true;
      }
      label_width += char_width;

    }

    // subscript keeps its bottom in line with the bottom of the bit chars,
    // superscript goes above the original char top by a quarter
    if( had_a_super ) {
      label_height *= 1.25;
    }
    label_height *= 1.2; // empirical
  }
#if 0
  // ****************************************************************************
  // draws the string centred on cds
  void MolDraw2DCairo::drawString( const std::string &str ,
                                 const std::pair<float,float> &cds ) {

    unsigned int fontSz=scale()*fontSize();


    float string_width , string_height;
    getStringSize( str , string_width , string_height );

    float draw_x = cds.first - string_width / 2.0;
    float draw_y = cds.second - string_height / 2.0;
    std::pair<float,float> draw_coords = getDrawCoords(std::make_pair(draw_x,draw_y));

    d_os<<"<svg:text";
    d_os<<" x='" << draw_coords.first;
    d_os<< "' y='" << draw_coords.second + fontSz <<"'"; // doesn't seem like this should be necessary, but vertical text alignment seems impossible
    d_os<<" style='font-size:"<<fontSz<<"px;font-style:normal;font-weight:normal;line-height:125%;letter-spacing:0px;word-spacing:0px;fill-opacity:1;stroke:none;font-family:sans-serif;text-anchor:start;"<<"fill:"<<col<<"'";
    d_os<<" >";

    int draw_mode = 0; // 0 for normal, 1 for superscript, 2 for subscript
    std::string span;
    bool first_span=true;
    for( int i = 0 , is = str.length() ; i < is ; ++i ) {
      // setStringDrawMode moves i along to the end of any <sub> or <sup>
      // markup
      if( '<' == str[i] && setStringDrawMode( str , draw_mode , i ) ) {
        if(!first_span){
          d_os<<span<<"</svg:tspan>";
          span="";
        }
        first_span=false;
        d_os<<"<svg:tspan";
        switch(draw_mode){
        case 1:
          d_os<<" style='baseline-shift:super;font-size:"<<fontSz*0.75<<"px;"<< "'"; break;
        case 2:
          d_os<<" style='baseline-shift:sub;font-size:"<<fontSz*0.75<<"px;"<< "'"; break;
        default:
          break;
        }
        d_os<<">";
        continue;
      }
      if(first_span){
        first_span=false;
        d_os<<"<svg:tspan>";
        span="";
      }
      span += str[i];
    }
    d_os<<span<<"</svg:tspan>";
    d_os<<"</svg:text>\n";
  }
#endif

} // EO namespace RDKit
