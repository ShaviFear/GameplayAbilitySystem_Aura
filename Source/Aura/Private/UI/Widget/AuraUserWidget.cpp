 // Copyright Shavi


#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UObject* InWidgetContoller)
{
	WidgetController = InWidgetContoller;
	WidgetControllerSet();
}
