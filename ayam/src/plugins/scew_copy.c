
#include "scew.h"
#include "xelement.h"

typedef int scew_bool;
#define SCEW_TRUE 1;
#define SCEW_FALSE 0;

static scew_bool copy_children_ (scew_element *new_element,
                                 scew_element const *element);
static scew_bool copy_attributes_ (scew_element *new_element,
                                   scew_element const *element);

scew_element*
scew_element_copy (scew_element const *element)
{
  scew_element *new_elem = NULL;

  new_elem = calloc (1, sizeof (scew_element));

  if (new_elem != NULL)
    {
      scew_bool copied =
        ((NULL == element->contents)
         || (scew_element_set_contents (new_elem, element->contents) != NULL));

      copied = copied
        && (scew_element_set_name (new_elem, element->name) != NULL)
        && copy_children_ (new_elem, element)
        && copy_attributes_ (new_elem, element);

      if (!copied)
        {
          scew_element_free (new_elem);
          new_elem = NULL;
        }

      if (!new_elem->attributes)
        new_elem->attributes = attribute_list_create();
    }

  return new_elem;
}


scew_attribute*
scew_attribute_copy (scew_attribute const *attribute)
{
  scew_attribute *new_attr = NULL;

  new_attr = calloc (1, sizeof (scew_attribute));

  if (new_attr != NULL)
    {
      scew_bool copied =
        (scew_attribute_set_name (new_attr, attribute->name) != NULL)
        && (scew_attribute_set_value (new_attr, attribute->value) != NULL);

      if (!copied)
        {
          scew_attribute_free (new_attr);
          new_attr = NULL;
        }
    }

  return new_attr;
}

scew_bool
copy_children_ (scew_element *new_element, scew_element const *element)
{
  scew_bool copied = SCEW_TRUE;
  scew_element *child = NULL;

  while((child = scew_element_next(element, child)) != NULL)
    {
      scew_element *new_child = scew_element_copy (child);
      copied = ((new_child != NULL)
         && (scew_element_add_elem (new_element, new_child) != NULL));
      if(!copied)
	break;
    }

  return copied;
}

scew_bool
copy_attributes_ (scew_element *new_element, scew_element const *element)
{
  scew_bool copied = SCEW_TRUE;
  scew_attribute *attribute = NULL;

  while((attribute = scew_attribute_next(element, attribute)) != NULL)
    {
      scew_attribute *new_attr = scew_attribute_copy (attribute);
      copied =
        ((new_attr != NULL)
         && (scew_element_add_attr (new_element, new_attr) != NULL));
      if(!copied)
	break;
    }

  return copied;
}
