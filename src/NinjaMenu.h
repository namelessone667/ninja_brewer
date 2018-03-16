#ifndef NinjaMenu_h
#define NinjaMenu_h

#define MENU_LABEL       const __FlashStringHelper*

enum NinjaMenuNavigation
{
  NINJAMENU_LEFT,
  NINJAMENU_RIGHT,
  NINJAMENU_ENTER,
  NINJAMENU_BACK,
  NINJAMENU_NONE
};

class INinjaMenuNavigationHandler
{
public:
  virtual NinjaMenuNavigation ScanNavigationButtons() = 0;
};

class NinjaMenuItem
{
public:
  virtual void DrawMenuItem(String& buffer) = 0;
  virtual NinjaMenuItem* ExecuteAction(NinjaMenuNavigation navigationStep) = 0;
  virtual void SaveChanges() = 0;
  virtual void DiscardChanges() = 0;

  NinjaMenuItem(MENU_LABEL menuItemLabel)
  {
    m_label = menuItemLabel;
  }

  MENU_LABEL GetLabel()
  {
    return m_label;
  }

  NinjaMenuItem* GetNextItem()
  {
    return m_next;
  }

  NinjaMenuItem* GetPreviousItem()
  {
    return m_previous;
  }

  NinjaMenuItem* SetParentItem(NinjaMenuItem* parent)
  {
    m_parent = parent;
    return this;
  }

  NinjaMenuItem* SetNextItem(NinjaMenuItem* nextItem)
  {
    if(nextItem == NULL)
      return this;

    if(m_next == NULL)
    {
      m_next = nextItem->SetPreviousItem(this);
    }
    else
    {
      NinjaMenuItem* oldNext = m_next;
      m_next = nextItem->SetPreviousItem(this)->SetNextItem(oldNext);
    }

    return this;
  }

protected:
  NinjaMenuItem* SetPreviousItem(NinjaMenuItem* previousItem)
  {
    if(previousItem == NULL)
      return this;
    m_previous = previousItem;
    return this;
  }

  NinjaMenuItem* m_parent = NULL;
  NinjaMenuItem* m_next = NULL;
  NinjaMenuItem* m_previous = NULL;
  MENU_LABEL m_label;
};

class SubNinjaMenuItem : public NinjaMenuItem
{
public:
  SubNinjaMenuItem(MENU_LABEL menuItemLabel)
    : NinjaMenuItem(menuItemLabel)
  {

  }

  SubNinjaMenuItem* AddSubMenu(NinjaMenuItem* menuItem)
  {
    if(menuItem == NULL)
      return this;

    if(m_first == NULL)
    {
      m_first = menuItem;
      m_current = menuItem;
    }
    else
    {
      NinjaMenuItem* item = m_current;
      while(item->GetNextItem() != NULL)
      {
        item = item->GetNextItem();
      }
      item->SetNextItem(menuItem);
    }
    menuItem->SetParentItem(this);
    return this;
  }

  NinjaMenuItem* ExecuteAction(NinjaMenuNavigation navigationStep)
  {
    switch(navigationStep)
    {
      case NINJAMENU_LEFT:
        if(m_current->GetPreviousItem() != NULL)
          m_current = m_current->GetPreviousItem();
          return this;
      case NINJAMENU_RIGHT:
        if(m_current->GetNextItem() != NULL)
          m_current = m_current->GetNextItem();
          return this;
      case NINJAMENU_ENTER:
        return m_current;
      case NINJAMENU_BACK:
        if(m_parent != NULL)
          return m_parent;
        else
          return this;
      default:
        return this;
    }
  }

  void DrawMenuItem(String& buffer)
  {
    buffer.concat(m_label);
    buffer.concat('\n');
    if(m_current != NULL)
    {
      buffer.concat("->");
      buffer.concat(m_current->GetLabel());
      buffer.concat('\n');
    }
  }

  void SaveChanges()
  {
    NinjaMenuItem* iterator = m_first;
    while(iterator != NULL)
    {
      iterator->SaveChanges();
      iterator = iterator->GetNextItem();
    }
  }

  void DiscardChanges()
  {
    NinjaMenuItem* iterator = m_first;
    while(iterator != NULL)
    {
      iterator->DiscardChanges();
      iterator = iterator->GetNextItem();
    }
  }
private:
  NinjaMenuItem* m_current = NULL;
  NinjaMenuItem* m_first = NULL;
};

template<typename T>
class BindedPropertyNinjaMenuItem : public NinjaMenuItem
{
  static_assert(std::is_arithmetic<T>::value, "BindedPropertyNinjaMenuItem can be used only with arithmetic types");
public:
  BindedPropertyNinjaMenuItem(MENU_LABEL menuItemLabel, Property<T>& property, T minValue, T maxValue, T step)
  : NinjaMenuItem(menuItemLabel), m_bindedProperty(property)
  {
    m_value = property.Get();
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_step = step;
  }

  NinjaMenuItem* ExecuteAction(NinjaMenuNavigation navigationStep)
  {
    switch(navigationStep)
    {
      case NINJAMENU_LEFT:
        if(m_value - m_step >= m_minValue)
          m_value -= m_step;
        return this;
      case NINJAMENU_RIGHT:
        if(m_value + m_step <= m_maxValue)
          m_value += m_step;
        return this;
      case NINJAMENU_ENTER:
      case NINJAMENU_BACK:
        if(navigationStep == NINJAMENU_BACK)
          m_value = m_bindedProperty.Get();
        if(m_parent != NULL)
          return m_parent;
        else
          return this;
      default:
        return this;
    }
  }

  void DrawMenuItem(String& buffer)
  {
    buffer.concat(m_label);
    buffer.concat('\n');
    buffer.concat('[');
    buffer.concat(m_minValue);
    buffer.concat("] ");
    buffer.concat(m_value);
    buffer.concat(" [");
    buffer.concat(m_maxValue);
    buffer.concat(']');
    buffer.concat('\n');
  }

  void SaveChanges()
  {
    m_bindedProperty.Set(m_value);
  }

  void DiscardChanges()
  {
    m_value = m_bindedProperty.Get();
  }

private:
  Property<T>& m_bindedProperty;
  T m_value;
  T m_minValue;
  T m_maxValue;
  T m_step;
};

template<>
class BindedPropertyNinjaMenuItem<bool> : public NinjaMenuItem
{
public:
  BindedPropertyNinjaMenuItem(MENU_LABEL menuItemLabel, Property<bool>& property)
  : NinjaMenuItem(menuItemLabel), m_bindedProperty(property)
  {
    m_value = property.Get();
  }

  NinjaMenuItem* ExecuteAction(NinjaMenuNavigation navigationStep)
  {
    switch(navigationStep)
    {
      case NINJAMENU_LEFT:
      case NINJAMENU_RIGHT:
        m_value = !m_value;
        return this;
      case NINJAMENU_ENTER:
      case NINJAMENU_BACK:
        if(navigationStep == NINJAMENU_BACK)
          m_value = m_bindedProperty.Get();
        if(m_parent != NULL)
          return m_parent;
        else
          return this;
      default:
        return this;
    }
  }

  void SaveChanges()
  {
    m_bindedProperty.Set(m_value);
  }

  void DiscardChanges()
  {
    m_value = m_bindedProperty.Get();
  }

  void DrawMenuItem(String& buffer)
  {
    buffer.concat(m_label);
    buffer.concat('\n');
    buffer.concat(m_value ? "ON" : "OFF");
    buffer.concat('\n');
  }

private:
  Property<bool>& m_bindedProperty;
  bool m_value;
};

class NinjaMenu
{
public:
  NinjaMenu (LiquidCrystal_I2C* lcd, unsigned int cols, unsigned int rows, INinjaMenuNavigationHandler* navigationProvider)
  {
    m_lcd = lcd;
    m_rows = rows;
    m_cols = cols;
    m_navigationProvider = navigationProvider;
  }
  void Begin()
  {
    m_lcd->begin(m_cols,m_rows);  //  LCD size
    m_lcd->setBacklight(HIGH);
    m_lcd->noCursor();
  }
  void DrawMenu()
  {
      if(m_current == NULL)
        return;

      NinjaMenuNavigation navigationStep = m_navigationProvider->ScanNavigationButtons();

      if(navigationStep != NINJAMENU_NONE)
      {
        NavigateMenu(navigationStep);
        DrawMenu();
        return;
      }

      String buffer;
      m_current->DrawMenuItem(buffer);

      DrawUsrScreen(buffer);
  }

  void NavigateMenu(NinjaMenuNavigation navigationStep)
  {
    if(m_current != NULL)
      m_current = m_current->ExecuteAction(navigationStep);
  }

  void SetRootMenuItem(SubNinjaMenuItem* rootMenuItem)
  {
      m_current = m_root = rootMenuItem;
  }

  void DrawUsrScreen(String buffer)
  {
    unsigned int current_row = 0;
    m_lcd->setCursor(0,0);
    while(current_row < m_rows)
    {
      int lineEnd = buffer.indexOf('\n');
      if(lineEnd == -1)
      {
        m_lcd->println("----");
      }
      else
      {
        m_lcd->println(buffer.substring(0, lineEnd < (int)m_cols ? lineEnd : m_cols - 1));
        buffer = buffer.substring(lineEnd+1);
      }
      current_row++;
    }
  }

  void SaveChanges()
  {
    if(m_root != NULL)
      m_root->SaveChanges();
  }

  void DiscardChanges()
  {
    if(m_root != NULL)
      m_root->DiscardChanges();
  }

  void Reset()
  {
    m_current = m_root;
  }
private:
  NinjaMenuItem* m_root = NULL;
  NinjaMenuItem* m_current = NULL;
  INinjaMenuNavigationHandler* m_navigationProvider = NULL;
  LiquidCrystal_I2C* m_lcd;
  unsigned int m_cols;
  unsigned int m_rows;
};

#endif
