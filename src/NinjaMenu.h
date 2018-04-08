#ifndef NinjaMenu_h
#define NinjaMenu_h

#include <list>

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
  virtual void Reset() = 0;

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

  void Reset()
  {
    m_current = m_first;
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
  BindedPropertyNinjaMenuItem(MENU_LABEL menuItemLabel, Property<T>& property, T minValue, T maxValue, T step, int precision)
  : NinjaMenuItem(menuItemLabel), m_bindedProperty(property)
  {
    m_value = property.Get();
    m_tempValue = m_value;
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_step = step;
    m_precision = precision;
  }

  NinjaMenuItem* ExecuteAction(NinjaMenuNavigation navigationStep)
  {
    switch(navigationStep)
    {
      case NINJAMENU_LEFT:
        if(m_tempValue - m_step >= m_minValue)
          m_tempValue -= m_step;
        return this;
      case NINJAMENU_RIGHT:
        if(m_tempValue + m_step <= m_maxValue)
          m_tempValue += m_step;
        return this;
      case NINJAMENU_ENTER:
      case NINJAMENU_BACK:
        if(navigationStep == NINJAMENU_BACK)
          m_tempValue = m_value;
        else
          m_value = m_tempValue;
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
    DrawVariable(buffer, m_minValue);
    buffer.concat("] ");
    DrawVariable(buffer, m_tempValue);
    buffer.concat(" [");
    DrawVariable(buffer, m_maxValue);
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
    m_tempValue = m_value;
  }

  void Reset()
  {
    DiscardChanges();
  }

private:
  void DrawVariable(String& buffer, int var)
  {
    buffer.concat(String::format("%.*d",m_precision, var));
  }

  void DrawVariable(String& buffer, double var)
  {
    buffer.concat(String::format("%.*f",m_precision, var));
  }
  Property<T>& m_bindedProperty;
  T m_value;
  T m_tempValue;
  T m_minValue;
  T m_maxValue;
  T m_step;
  int m_precision;
};

template<>
class BindedPropertyNinjaMenuItem<bool> : public NinjaMenuItem
{
public:
  BindedPropertyNinjaMenuItem(MENU_LABEL menuItemLabel, Property<bool>& property)
  : NinjaMenuItem(menuItemLabel), m_bindedProperty(property)
  {
    m_value = property.Get();
    m_tempValue = m_value;
  }

  NinjaMenuItem* ExecuteAction(NinjaMenuNavigation navigationStep)
  {
    switch(navigationStep)
    {
      case NINJAMENU_LEFT:
      case NINJAMENU_RIGHT:
        m_tempValue = !m_tempValue;
        return this;
      case NINJAMENU_ENTER:
      case NINJAMENU_BACK:
        if(navigationStep == NINJAMENU_BACK)
          m_tempValue = m_value;
        else
          m_value = m_tempValue;
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
    m_tempValue = m_value;
  }

  void Reset()
  {
    DiscardChanges();
  }

  void DrawMenuItem(String& buffer)
  {
    buffer.concat(m_label);
    buffer.concat('\n');
    buffer.concat(m_tempValue ? "ON" : "OFF");
    buffer.concat('\n');
  }

private:
  Property<bool>& m_bindedProperty;
  bool m_value;
  bool m_tempValue;
};

template<class C>
class CommandNinjaMenuItem : public NinjaMenuItem
{
public:
  CommandNinjaMenuItem(MENU_LABEL menuItemLabel, C& command)
  : NinjaMenuItem(menuItemLabel), m_command(command)
  {

  }

  NinjaMenuItem* ExecuteAction(NinjaMenuNavigation navigationStep)
  {
    switch(navigationStep)
    {
      case NINJAMENU_LEFT:
      case NINJAMENU_RIGHT:
        return this;
      case NINJAMENU_ENTER:
        m_command.Execute();
        return m_parent;
      case NINJAMENU_BACK:
        return m_parent;
      default:
        return this;
    }
  }

  void SaveChanges()
  {

  }

  void DiscardChanges()
  {

  }

  void Reset()
  {

  }

  void DrawMenuItem(String& buffer)
  {
    buffer.concat(m_label);
    buffer.concat('\n');
    buffer.concat("[Confirm] to run.");
    buffer.concat('\n');
  }

private:
  C& m_command;
};

template<typename T>
class OptionsPropertyNinjaMenuItem : public NinjaMenuItem
{
public:
  OptionsPropertyNinjaMenuItem(MENU_LABEL menuItemLabel, Property<T>& property, T defaultValue)
  : NinjaMenuItem(menuItemLabel), m_bindedProperty(property)
  {
    m_value = defaultValue;
    m_tempValue = defaultValue;
  }

  OptionsPropertyNinjaMenuItem* AddOption(T option, String label)
  {
    m_options[option] = label;
    if(m_bindedProperty.Get() == option)
    {
      m_value = option;
      m_tempValue = option;
    }
    return this;
  }

  NinjaMenuItem* ExecuteAction(NinjaMenuNavigation navigationStep)
  {
    auto i = m_options.find(m_tempValue);
    switch(navigationStep)
    {
      case NINJAMENU_LEFT:
      case NINJAMENU_RIGHT:
        if(i == m_options.end())
        {
          if(m_options.empty() == false)
            m_tempValue = m_options.begin()->first;

          return this;
        }
        if(navigationStep == NINJAMENU_LEFT)
        {
          if(i != m_options.begin())
            m_tempValue = (--i)->first;
          else
            m_tempValue = (--(m_options.end()))->first;
        }
        else if(navigationStep == NINJAMENU_RIGHT)
        {
          if(i != --(m_options.end()))
            m_tempValue = (++i)->first;
          else
            m_tempValue = m_options.begin()->first;
        }
        return this;
      case NINJAMENU_ENTER:
      case NINJAMENU_BACK:
        if(navigationStep == NINJAMENU_BACK)
          m_tempValue = m_value;
        else
          m_value = m_tempValue;
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
    m_tempValue = m_value;
  }

  void Reset()
  {
    DiscardChanges();
  }

  void DrawMenuItem(String& buffer)
  {
    auto i = m_options.find(m_tempValue);
    buffer.concat(m_label);
    buffer.concat('\n');
    buffer.concat(i == m_options.end() ? "UNDEFINED" : i->second);
    buffer.concat('\n');
  }

private:
  Property<T>& m_bindedProperty;
  T m_value;
  T m_tempValue;
  std::map<T, String> m_options;
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
    {
      NinjaMenuItem* newItem = m_current->ExecuteAction(navigationStep);
      if(newItem != NULL)
        m_current = newItem;
    }
  }

  void SetRootMenuItem(SubNinjaMenuItem* rootMenuItem)
  {
      m_current = m_root = rootMenuItem;
  }

  void DrawUsrScreen(String buffer)
  {
    unsigned int current_row = 0;

    while(current_row < m_rows)
    {
      int eol = buffer.indexOf('\n');
      PrintLineToLCD(current_row++, buffer.substring(0, eol == -1 ? buffer.length() : eol));
      buffer = buffer.substring(eol+1);
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
    m_current->Reset();
  }
private:
  void PrintLineToLCD(int lineNumber, const String& line)
  {
    m_lcd->setCursor(0,lineNumber);
    int length = line.length();
    m_lcd->print(line.substring(0, length <= (int)m_cols ? length : m_cols));
    for(int i = length; i < m_cols; i++)
      m_lcd->write(' ');
  }

  NinjaMenuItem* m_root = NULL;
  NinjaMenuItem* m_current = NULL;
  INinjaMenuNavigationHandler* m_navigationProvider = NULL;
  LiquidCrystal_I2C* m_lcd;
  unsigned int m_cols;
  unsigned int m_rows;
};

class INinjaCommand
{
public:
  virtual void Execute() = 0;
};

class NinjaMenuCommand : public INinjaCommand
{
public:
  NinjaMenuCommand(NinjaMenu& menu) :
    m_menu(menu)
  {

  }
protected:
  NinjaMenu& m_menu;
};

class SaveChangesNinjaMenuCommand : public NinjaMenuCommand
{
public:
  SaveChangesNinjaMenuCommand(NinjaMenu& menu) : NinjaMenuCommand(menu)
  {

  }

  void Execute()
  {
    m_menu.SaveChanges();
  }
};

class DiscardChangesNinjaMenuCommand : public NinjaMenuCommand
{
public:
  DiscardChangesNinjaMenuCommand(NinjaMenu& menu) : NinjaMenuCommand(menu)
  {

  }

  void Execute()
  {
    m_menu.DiscardChanges();
  }
};

class NinjaCommandChain : INinjaCommand
{
public:
  NinjaCommandChain()
  {

  }

  NinjaCommandChain* AddCommand(INinjaCommand* command)
  {
    m_commands.push_back(command);
    return this;
  }

  void Execute()
  {
    for(auto it = m_commands.begin(); it != m_commands.end(); ++it)
    {
      (*it)->Execute();
    }
  }
private:
  std::list<INinjaCommand*> m_commands;
};

#endif
