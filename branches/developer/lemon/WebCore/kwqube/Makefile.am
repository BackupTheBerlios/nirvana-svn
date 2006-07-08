# Process this file with automake to produce Makefile.in
NULL=
AM_CPPFLAGS = \
	@DEFS@\
	-DPKGDATADIR=\"$(pkgdatadir)/\"\
	-I$(top_srcdir)/khtml \
	-I$(top_srcdir)/khtml/dom \
	-I$(top_srcdir)/khtml/css \
	-I$(top_srcdir)/khtml/html \
	-I$(top_srcdir)/khtml/misc/ \
	-I$(top_srcdir)/khtml/rendering/ \
	-I$(top_srcdir)/khtml/xml \
	-I$(top_srcdir)/khtml/ecma/ \
	-I$(top_srcdir)/khtml/editing/ \
	-I$(top_srcdir)/kwiq/ \
	-I$(top_srcdir)/ForwardingHeaders
	$(NULL)

KWQCharsetData.c: 
	perl make-charset-table.pl character-sets.txt mac-encodings.txt > $@

KWQColorData.c: 
	gperf -CDEot -L 'ANSI-C' -k '*' -N findColor KWQColorData.gperf > $@ 

KWIQCFEncodings.h: 
	perl make-charset-table.pl character-sets.txt mac-encodings.txt -e >$@

libnrcore_kwiq_gtk_cpp_sources=\
KWIQMacros.cpp \
WebCoreBridge.cpp \
WebCoreHistory.cpp \
WebCoreSettings.cpp \
WebCoreImageRendererFactory.cpp\
WebCoreTextRenderer.cpp \
WebCoreTextRendererFactory.cpp \
WebCoreViewFactory.cpp \
KWQAccObject.cpp \
KWQAccObjectCache.cpp \
KWQApplication.cpp \
KWQArrayImpl.cpp \
KWQAssertions.cpp \
KWQBuffer.cpp \
KWQCString.cpp \
KWQCollection.cpp \
KWQColor.cpp \
KWQColorGroup.cpp \
KWQCompletion.cpp \
KWQDOMNode.cpp \
KWQDictImpl.cpp \
KWQDrawUtil.cpp \
KWQFontFamily.cpp \
KWQFontInfo.cpp \
KWQFoundationExtras.h\
KWQGlobal.cpp \
KWQGuardedPtr.cpp \
KWQKCharsets.cpp \
KWQKConfigBase.cpp \
KWQKCookieJar.cpp \
KWQKFileItem.cpp \
KWQKGlobal.cpp \
KWQKGlobalSettings.cpp \
KWQKHTMLFactory.cpp \
KWQKHTMLPartBrowserExtension.cpp \
KWQKHTMLSettings.cpp \
KWQKHTMLView.cpp \
KWQKHistoryProvider.cpp \
KWQKIONetAccess.cpp \
KWQKIconLoader.cpp \
KWQKJavaAppletWidget.cpp \
KWQKJob.cpp \
KWQKJobClasses.cpp \
KWQKLocale.cpp \
KWQKMimeType.cpp \
KWQKPartsBrowserInterface.cpp \
KWQKPartsEvent.cpp \
KWQKPartsPart.cpp \
KWQKSSLKeyGen.cpp \
KWQKSimpleConfig.cpp \
KWQKStringHandler.cpp \
KWQKURL.cpp \
KWQKWin.cpp \
KWQListImpl.cpp \
KWQLoader.cpp \
KWQLogging.cpp \
KWQMapImpl.cpp \
KWQMovie.cpp\
KWQObject.cpp \
KWQPageState.cpp \
KWQPalette.cpp \
KWQPen.cpp \
KWQPoint.cpp \
KWQPointArray.cpp \
KWQPtrDictImpl.cpp \
KWQRect.cpp \
KWQRegExp.cpp \
KWQRegion.cpp \
KWQRenderTreeDebug.cpp \
KWQResourceLoader.cpp \
KWQSignal.cpp \
KWQSignalStubs.cpp \
KWQSize.cpp \
KWQSlot.cpp \
KWQString.cpp \
KWQStringList.cpp \
KWQStyle.cpp \
KWQTextStream.cpp \
KWQValueListImpl.cpp \
KWQVariant.cpp \
KWQVectorImpl.cpp \
KWQXmlAttributes.cpp \
KWQXmlDefaultHandler.cpp \
KWQXmlSimpleReader.cpp \
WebCoreCache.cpp\
WebCoreUnicode.cpp \
cxx.cpp \
KWQButton.cpp \
KWQButtonGroup.cpp \
KWQChar.cpp \
KWQCharsets.cpp \
KWQCheckBox.cpp \
KWQComboBox.cpp \
KWQCursor.cpp \
KWQDateTime.cpp \
KWQEvent.cpp \
KWQFile.cpp \
KWQFileButton.cpp \
KWQFileDialog.cpp \
KWQFont.cpp \
KWQFontMetrics.cpp \
KWQFrame.cpp \
KWQKCursor.cpp \
KWQKHTMLPart.cpp \
KWQKStandardDirs.cpp \
KWQKWinModule.cpp \
KWQLabel.cpp \
KWQLineEdit.cpp \
KWQListBox.cpp \
KWQPaintDeviceMetrics.cpp \
KWQPainter.cpp \
KWQPixmap.cpp \
KWQPushButton.cpp \
KWQRadioButton.cpp \
KWQScrollBar.cpp \
KWQScrollView.cpp \
KWQTextCodec.cpp \
KWQTextEdit.cpp \
KWQTimer.cpp \
KWQWidget.cpp \
KWQWindowWidget.cpp \
$(NULL)

libnrcore_kwiq_gtk_h_private_sources=\
GObjectMisc.h\
KWIQCFEncodings.h \
KWIQMacros.h \
KWQAccObject.h \
KWQAccObjectCache.h \
KWQApplication.h \
KWQArrayImpl.h \
KWQAssertions.h \
KWQAsyncIO.h \
KWQBrush.h \
KWQBuffer.h \
KWQButton.h \
KWQButtonGroup.h \
KWQCString.h \
KWQCharsets.h \
KWQCheckBox.h \
KWQCollection.h \
KWQColor.h \
KWQComboBox.h \
KWQCompletion.h \
KWQCursor.h \
KWQDCOPClient.h \
KWQDOMNode.h \
KWQDateTime.h \
KWQDef.h \
KWQDict.h \
KWQDictImpl.h \
KWQDrawUtil.h \
KWQEvent.h \
KWQExceptions.h \
KWQFile.h \
KWQFileButton.h \
KWQFont.h \
KWQFontFamily.h \
KWQFontInfo.h \
KWQFontMetrics.h \
KWQFrame.h \
KWQGuardedPtr.h \
KWQIODevice.h \
KWQImage.h \
KWQKApplication.h \
KWQKCharsets.h \
KWQKComboBox.h \
KWQKCompletionBox.h \
KWQKConfigBase.h \
KWQKCookieJar.h \
KWQKCursor.h \
KWQKDebug.h \
KWQKFileDialog.h \
KWQKFileItem.h \
KWQKGlobal.h \
KWQKGlobalSettings.h \
KWQKHTMLFactory.h \
KWQKHTMLFind.h \
KWQKHTMLPageCache.h \
KWQKHTMLPart.h \
KWQKHTMLPartBrowserExtension.h \
KWQKHTMLSettings.h \
KWQKIOGlobal.h \
KWQKIONetAccess.h \
KWQKIconLoader.h \
KWQKImageIO.h \
KWQKInstance.h \
KWQKJavaAppletContext.h \
KWQKJavaAppletWidget.h \
KWQKJob.h \
KWQKJobClasses.h \
KWQKKAction.h \
KWQKLibrary.h \
KWQKLineEdit.h \
KWQKListBox.h \
KWQKLocale.h \
KWQKMimeType.h \
KWQKPartsBrowserExtension.h \
KWQKPartsBrowserInterface.h \
KWQKPartsEvent.h \
KWQKPartsFactory.h \
KWQKPartsHistoryProvider.h \
KWQKPartsPart.h \
KWQKPartsPartManager.h \
KWQKSSLKeyGen.h \
KWQKSimpleConfig.h \
KWQKStandardDirs.h \
KWQKStaticDeleter.h \
KWQKStringHandler.h \
KWQKURL.h \
KWQKWin.h \
KWQKWinModule.h \
KWQLabel.h \
KWQLineEdit.h \
KWQListBox.h \
KWQListImpl.h \
KWQLoader.h \
KWQLogging.h \
KWQMap.h \
KWQMapImpl.h \
KWQMemArray.h \
KWQMovie.h\
KWQNamespace.h \
KWQObject.h \
KWQPageState.h \
KWQPaintDevice.h \
KWQPaintDeviceMetrics.h \
KWQPainter.h \
KWQPalette.h \
KWQPen.h \
KWQPixmap.h \
KWQPointArray.h \
KWQPrinter.h \
KWQPtrDict.h \
KWQPtrDictImpl.h \
KWQPtrList.h \
KWQPtrQueue.h \
KWQPtrStack.h \
KWQPtrVector.h \
KWQPushButton.h \
KWQRadioButton.h \
KWQRect.h \
KWQRefPtr.h \
KWQRegExp.h \
KWQRegion.h \
KWQRenderTreeDebug.h \
KWQResourceLoader.h \
KWQScrollBar.h \
KWQScrollView.h \
KWQSignal.h \
KWQSize.h \
KWQSlot.h \
KWQSortedList.h \
KWQString.h \
KWQStringList.h \
KWQStyle.h \
KWQStyleSheet.h \
KWQTextArea.h \
KWQTextCodec.h \
KWQTextEdit.h \
KWQTextField.h \
KWQTextStream.h \
KWQTimer.h \
KWQToolTip.h \
KWQValueList.h \
KWQValueListImpl.h \
KWQVariant.h \
KWQVectorImpl.h \
KWQWidget.h \
KWQWindowWidget.h \
KWQXmlAttributes.h \
KWQXmlDefaultHandler.h \
KWQXmlSimpleReader.h \
GLibHelpers.h\
cstddef\
new \
ostream\
algorithm \
$(NULL)

libnrcore_kwiq_gtk_h_public_sources=\
WebCoreBridge.h \
WebCoreCache.h\
WebCoreFoundation.h \
WebCoreHistory.h \
WebCoreImageRenderer.h \
WebCoreImageRendererFactory.h \
WebCoreResourceLoader.h \
WebCoreTextRenderer.h \
WebCoreTextRendererFactory.h \
WebCoreUnicode.h \
WebCoreViewFactory.h\
WebCoreSettings.h \
WebCoreKeyboardAccess.h \
KWIQCGContext.h \
KWIQPageCache.h \
KWIQResponse.h \
$(NULL)

libnrcore_kwiq_gtk_la_SOURCES = \
	$(libnrcore_kwiq_gtk_cpp_sources)\
	$(libnrcore_kwiq_gtk_h_private_sources)\
	$(NULL)

lib_LTLIBRARIES = libnrcore_kwiq_gtk.la

libnrcore_kwiq_gtkincludedir = $(includedir)/osb/NRCore/
libnrcore_kwiq_gtkinclude_HEADERS = $(libnrcore_kwiq_gtk_h_public_sources) 
libnrcore_kwiq_gtk_la_LDFLAGS = -version-info 0:0:0



EXTRA_DIST = $(BUILT_SOURCES) KWQColorData.gperf character-sets.txt mac-encodings.txt

BUILT_SOURCES =	KWQCharsetData.c KWQColorData.c KWIQCFEncodings.h

MAINTAINERCLEANFILES = $(BUILT_SOURCES) 

