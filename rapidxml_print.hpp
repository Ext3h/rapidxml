#ifndef RAPIDXML_PRINT_HPP_INCLUDED
#define RAPIDXML_PRINT_HPP_INCLUDED

// Copyright (C) 2006, 2009 Marcin Kalicinski
// Version 1.13
// Revision $DateTime: 2009/05/13 01:46:17 $
//! \file rapidxml_print.hpp This file contains rapidxml printer implementation

#include "rapidxml.hpp"

// Only include streams if not disabled
#ifndef RAPIDXML_NO_STREAMS
    #include <ostream>
    #include <iterator>
#endif

namespace rapidxml
{

    ///////////////////////////////////////////////////////////////////////
    // Printing flags

    const int print_no_indenting = 0x1;   //!< Printer flag instructing the printer to suppress indenting of XML. See print() function.

    ///////////////////////////////////////////////////////////////////////
    // Internal

    //! \cond internal
    namespace internal
    {
        
        ///////////////////////////////////////////////////////////////////////////
        // Internal character operations
    
        // Copy characters from given range to given output iterator
        template<class OutIt, class Ch, int Flags>
        inline OutIt copy_chars(const Ch *begin, const Ch *end, OutIt out)
        {
            while (begin != end)
                *out++ = *begin++;
            return out;
        }
        
        // Copy characters from given range to given output iterator and expand
        // characters into references (&lt; &gt; &apos; &quot; &amp;)
        template<class OutIt, class Ch, int Flags>
        inline OutIt copy_and_expand_chars(const Ch *begin, const Ch *end, OutIt out)
        {
            while (begin != end)
            {
                switch (*begin)
                {
                case Ch('<'):
                    *out++ = Ch('&'); *out++ = Ch('l'); *out++ = Ch('t'); *out++ = Ch(';');
                    break;
                case Ch('>'): 
                    *out++ = Ch('&'); *out++ = Ch('g'); *out++ = Ch('t'); *out++ = Ch(';');
                    break;
                case Ch('\''): 
                    *out++ = Ch('&'); *out++ = Ch('a'); *out++ = Ch('p'); *out++ = Ch('o'); *out++ = Ch('s'); *out++ = Ch(';');
                    break;
                case Ch('"'): 
                    *out++ = Ch('&'); *out++ = Ch('q'); *out++ = Ch('u'); *out++ = Ch('o'); *out++ = Ch('t'); *out++ = Ch(';');
                    break;
                case Ch('&'): 
                    *out++ = Ch('&'); *out++ = Ch('a'); *out++ = Ch('m'); *out++ = Ch('p'); *out++ = Ch(';'); 
                    break;
                default:
                    *out++ = *begin;    // No expansion, copy character
                }
                ++begin;    // Step to next character
            }
            return out;
        }

        // Fill given output iterator with repetitions of the same character
        template<class OutIt, class Ch, int Flags>
        inline OutIt fill_chars(OutIt out, int n, Ch ch)
        {
            for (int i = 0; i < n; ++i)
                *out++ = ch;
            return out;
        }

        // Find character
        template<class Ch, Ch ch, int Flags>
        inline bool find_char(const Ch *begin, const Ch *end)
        {
            while (begin != end)
                if (*begin++ == ch)
                    return true;
            return false;
        }

        ///////////////////////////////////////////////////////////////////////////
        // Internal printing operations
    
        template<class OutIt, class Ch, int Flags>
        OutIt print_node(OutIt out, const xml_node<Ch>* node, int indent);
        template<class OutIt, class Ch, int Flags>
        OutIt print_children(OutIt out, const xml_node<Ch>* node, int indent);
        template<class OutIt, class Ch, int Flags>
        OutIt print_attributes(OutIt out, const xml_node<Ch>* node);
        template<class OutIt, class Ch, int Flags>
        OutIt print_data_node(OutIt out, const xml_node<Ch>* node, int indent);
        template<class OutIt, class Ch, int Flags>
        OutIt print_cdata_node(OutIt out, const xml_node<Ch>* node, int indent);
        template<class OutIt, class Ch, int Flags>
        OutIt print_element_node(OutIt out, const xml_node<Ch>* node, int indent);
        template<class OutIt, class Ch, int Flags>
        OutIt print_declaration_node(OutIt out, const xml_node<Ch>* node, int indent);
        template<class OutIt, class Ch, int Flags>
        OutIt print_comment_node(OutIt out, const xml_node<Ch>* node, int indent);
        template<class OutIt, class Ch, int Flags>
        OutIt print_doctype_node(OutIt out, const xml_node<Ch>* node, int indent);
        template<class OutIt, class Ch, int Flags>
        OutIt print_pi_node(OutIt out, const xml_node<Ch>* node, int indent);

        // Print node
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_node(OutIt out, const xml_node<Ch> *node, int indent)
        {
            // Print proper node type
            switch (node->type())
            {

            // Document
            case node_document:
                out = print_children<OutIt, Ch, Flags>(out, node, indent);
                break;

            // Element
            case node_element:
                out = print_element_node<OutIt, Ch, Flags>(out, node, indent);
                break;
            
            // Data
            case node_data:
                out = print_data_node<OutIt, Ch, Flags>(out, node, indent);
                break;
            
            // CDATA
            case node_cdata:
                out = print_cdata_node<OutIt, Ch, Flags>(out, node, indent);
                break;

            // Declaration
            case node_declaration:
                out = print_declaration_node<OutIt, Ch, Flags>(out, node, indent);
                break;

            // Comment
            case node_comment:
                out = print_comment_node<OutIt, Ch, Flags>(out, node, indent);
                break;
            
            // Doctype
            case node_doctype:
                out = print_doctype_node<OutIt, Ch, Flags>(out, node, indent);
                break;

            // Pi
            case node_pi:
                out = print_pi_node<OutIt, Ch, Flags>(out, node, indent);
                break;

                // Unknown
            default:
                assert(0);
                break;
            }
            
            // If indenting not disabled, add line break after node
            if constexpr (!(Flags & print_no_indenting))
                *out = Ch('\n'), ++out;

            // Return modified iterator
            return out;
        }
        
        // Print children of the node                               
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_children(OutIt out, const xml_node<Ch> *node, int indent)
        {
            for (xml_node<Ch> *child = node->first_node(); child; child = child->next_sibling())
                out = print_node<OutIt, Ch, Flags>(out, child, indent);
            return out;
        }

        // Print attributes of the node
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_attributes(OutIt out, const xml_node<Ch> *node)
        {
            for (xml_attribute<Ch> *attribute = node->first_attribute(); attribute; attribute = attribute->next_attribute())
            {
                if (attribute->name() && attribute->value())
                {
                    // Print attribute name
                    *out = Ch(' '), ++out;
                    out = copy_chars<OutIt, Ch, Flags>(attribute->name(), attribute->name() + attribute->name_size(), out);
                    *out = Ch('='), ++out;
                    // Print attribute value
                    *out = Ch('"'), ++out;
                    out = copy_and_expand_chars<OutIt, Ch, Flags>(attribute->value(), attribute->value() + attribute->value_size(), out);
                    *out = Ch('"'), ++out;
                }
            }
            return out;
        }

        // Print data node
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_data_node(OutIt out, const xml_node<Ch> *node, int indent)
        {
            assert(node->type() == node_data);
            if constexpr (!(Flags & print_no_indenting))
                out = fill_chars<OutIt, Ch, Flags>(out, indent, Ch('\t'));
            out = copy_and_expand_chars<OutIt, Ch, Flags>(node->value(), node->value() + node->value_size(), out);
            return out;
        }

        // Print data node
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_cdata_node(OutIt out, const xml_node<Ch> *node, int indent)
        {
            assert(node->type() == node_cdata);
            if constexpr (!(Flags & print_no_indenting))
                out = fill_chars<OutIt, Ch, Flags>(out, indent, Ch('\t'));
            *out = Ch('<'); ++out;
            *out = Ch('!'); ++out;
            *out = Ch('['); ++out;
            *out = Ch('C'); ++out;
            *out = Ch('D'); ++out;
            *out = Ch('A'); ++out;
            *out = Ch('T'); ++out;
            *out = Ch('A'); ++out;
            *out = Ch('['); ++out;
            out = copy_chars<OutIt, Ch, Flags>(node->value(), node->value() + node->value_size(), out);
            *out = Ch(']'); ++out;
            *out = Ch(']'); ++out;
            *out = Ch('>'); ++out;
            return out;
        }

        // Print element node
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_element_node(OutIt out, const xml_node<Ch> *node, int indent)
        {
            assert(node->type() == node_element);

            // Print element name and attributes, if any
            if constexpr (!(Flags & print_no_indenting))
                out = fill_chars<OutIt, Ch, Flags>(out, indent, Ch('\t'));
            *out = Ch('<'), ++out;
            out = copy_chars<OutIt, Ch, Flags>(node->name(), node->name() + node->name_size(), out);
            out = print_attributes<OutIt, Ch, Flags>(out, node);
            
            // If node is childless
            if (node->value_size() == 0 && !node->first_node())
            {
                // Print childless node tag ending
                *out = Ch('/'), ++out;
                *out = Ch('>'), ++out;
            }
            else
            {
                // Print normal node tag ending
                *out = Ch('>'), ++out;

                // Test if node contains a single data node only (and no other nodes)
                xml_node<Ch> *child = node->first_node();
                if (!child)
                {
                    // If node has no children, only print its value without indenting
                    out = copy_and_expand_chars<OutIt, Ch, Flags>(node->value(), node->value() + node->value_size(), out);
                }
                else if (child->next_sibling() == 0 && child->type() == node_data)
                {
                    // If node has a sole data child, only print its value without indenting
                    out = copy_and_expand_chars<OutIt, Ch, Flags>(child->value(), child->value() + child->value_size(), out);
                }
                else
                {
                    // Print all children with full indenting
                    if constexpr (!(Flags & print_no_indenting))
                        *out = Ch('\n'), ++out;
                    out = print_children<OutIt, Ch, Flags>(out, node, indent + 1);
                    if constexpr (!(Flags & print_no_indenting))
                        out = fill_chars<OutIt, Ch, Flags>(out, indent, Ch('\t'));
                }

                // Print node end
                *out = Ch('<'), ++out;
                *out = Ch('/'), ++out;
                out = copy_chars<OutIt, Ch, Flags>(node->name(), node->name() + node->name_size(), out);
                *out = Ch('>'), ++out;
            }
            return out;
        }

        // Print declaration node
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_declaration_node(OutIt out, const xml_node<Ch> *node, int indent)
        {
            // Print declaration start
            if constexpr (!(Flags & print_no_indenting))
                out = fill_chars<OutIt, Ch, Flags>(out, indent, Ch('\t'));
            *out = Ch('<'), ++out;
            *out = Ch('?'), ++out;
            *out = Ch('x'), ++out;
            *out = Ch('m'), ++out;
            *out = Ch('l'), ++out;

            // Print attributes
            out = print_attributes<OutIt, Ch, Flags>(out, node);
            
            // Print declaration end
            *out = Ch('?'), ++out;
            *out = Ch('>'), ++out;
            
            return out;
        }

        // Print comment node
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_comment_node(OutIt out, const xml_node<Ch> *node, int indent)
        {
            assert(node->type() == node_comment);
            if constexpr (!(Flags & print_no_indenting))
                out = fill_chars<OutIt, Ch, Flags>(out, indent, Ch('\t'));
            *out = Ch('<'), ++out;
            *out = Ch('!'), ++out;
            *out = Ch('-'), ++out;
            *out = Ch('-'), ++out;
            out = copy_chars<OutIt, Ch, Flags>(node->value(), node->value() + node->value_size(), out);
            *out = Ch('-'), ++out;
            *out = Ch('-'), ++out;
            *out = Ch('>'), ++out;
            return out;
        }

        // Print doctype node
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_doctype_node(OutIt out, const xml_node<Ch> *node, int indent)
        {
            assert(node->type() == node_doctype);
            if constexpr (!(Flags & print_no_indenting))
                out = fill_chars<OutIt, Ch, Flags>(out, indent, Ch('\t'));
            *out = Ch('<'), ++out;
            *out = Ch('!'), ++out;
            *out = Ch('D'), ++out;
            *out = Ch('O'), ++out;
            *out = Ch('C'), ++out;
            *out = Ch('T'), ++out;
            *out = Ch('Y'), ++out;
            *out = Ch('P'), ++out;
            *out = Ch('E'), ++out;
            *out = Ch(' '), ++out;
            out = copy_chars<OutIt, Ch, Flags>(node->value(), node->value() + node->value_size(), out);
            *out = Ch('>'), ++out;
            return out;
        }

        // Print pi node
        template<class OutIt, class Ch, int Flags>
        inline OutIt print_pi_node(OutIt out, const xml_node<Ch> *node, int indent)
        {
            assert(node->type() == node_pi);
            if constexpr (!(Flags & print_no_indenting))
                out = fill_chars<OutIt, Ch, Flags>(out, indent, Ch('\t'));
            *out = Ch('<'), ++out;
            *out = Ch('?'), ++out;
            out = copy_chars<OutIt, Ch, Flags>(node->name(), node->name() + node->name_size(), out);
            *out = Ch(' '), ++out;
            out = copy_chars<OutIt, Ch, Flags>(node->value(), node->value() + node->value_size(), out);
            *out = Ch('?'), ++out;
            *out = Ch('>'), ++out;
            return out;
        }

    }
    //! \endcond

    ///////////////////////////////////////////////////////////////////////////
    // Printing

    //! Prints XML to given output iterator.
    //! \param out Output iterator to print to.
    //! \param node Node to be printed. Pass xml_document to print entire document.
    //! \param flags Flags controlling how XML is printed.
    //! \return Output iterator pointing to position immediately after last character of printed text.
    template<class OutIt, class Ch, int Flags = 0>
    inline OutIt print(OutIt out, const xml_node<Ch> &node = 0)
    {
        return internal::print_node<OutIt, Ch, Flags>(out, &node, 0);
    }

#ifndef RAPIDXML_NO_STREAMS

    //! Prints XML to given output stream.
    //! \param out Output stream to print to.
    //! \param node Node to be printed. Pass xml_document to print entire document.
    //! \param flags Flags controlling how XML is printed.
    //! \return Output stream.
    template<class Ch, int Flags = 0>
    inline std::basic_ostream<Ch> &print(std::basic_ostream<Ch> &out, const xml_node<Ch> &node)
    {
        print<std::ostream_iterator<Ch>, Ch>(std::ostream_iterator<Ch>(out), node);
        return out;
    }

    //! Prints formatted XML to given output stream. Uses default printing flags. Use print() function to customize printing process.
    //! \param out Output stream to print to.
    //! \param node Node to be printed.
    //! \return Output stream.
    template<class Ch, int Flags = 0>
    inline std::basic_ostream<Ch> &operator <<(std::basic_ostream<Ch> &out, const xml_node<Ch> &node)
    {
        return print<Ch, Flags>(out, node);
    }

#endif

}

#endif
