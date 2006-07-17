SUBDIRS = . dropin kdesrc src

SUFFIXES = .moc

bin_SCRIPTS = makemoc

moc_files = ./dropin/atheos/acheckbox.moc \
	./dropin/atheos/acombobox.moc \
	./dropin/atheos/aframe.moc \
	./dropin/atheos/alineedit.moc \
	./dropin/atheos/alistbox.moc \
	./dropin/atheos/amultilineedit.moc \
	./dropin/atheos/apushbutton.moc \
	./dropin/atheos/aradiobutton.moc \
	./dropin/atheos/ascrollview.moc \
	./dropin/atheos/atimer.moc \
	./dropin/atheos/awidget.moc \
	./dropin/atheos/old_aguardedptr.moc \
	./dropin/khtml/khtml_ext.moc \
	./dropin/kio/jobclasses.moc \
	./dropin/kio/krun.moc \
	./dropin/kparts/part.moc \
	./dropin/kuniqueapp.moc \
	./dropin/klibloader.moc \
	./dropin/dcopobject.moc \
	./dropin/kapp.moc \
	./dropin/kdialog.moc \
	./dropin/kdialogbase.moc \
	./kdesrc/khtml/domtreeview.moc \
	./kdesrc/khtml/ecma/kjs_debugwin.moc \
	./kdesrc/khtml/ecma/kjs_window.moc \
	./kdesrc/khtml/html/html_documentimpl.moc \
	./kdesrc/khtml/html/htmltokenizer.moc \
	./kdesrc/khtml/java/javaembed.moc \
	./kdesrc/khtml/java/kjavaapplet.moc \
	./kdesrc/khtml/java/kjavaappletcontext.moc \
	./kdesrc/khtml/java/kjavaappletserver.moc \
	./kdesrc/khtml/java/kjavaappletwidget.moc \
	./kdesrc/khtml/java/kjavadownloader.moc \
	./kdesrc/khtml/java/kjavaprocess.moc \
	./kdesrc/khtml/khtml_factory.moc \
	./kdesrc/khtml/khtml_pagecache.moc \
	./kdesrc/khtml/khtml_part.moc \
	./kdesrc/khtml/khtml_run.moc \
	./kdesrc/khtml/khtmlimage.moc \
	./kdesrc/khtml/khtmlview.moc \
	./kdesrc/khtml/misc/loader.moc \
	./kdesrc/khtml/misc/loader_atheos.moc \
	./kdesrc/khtml/rendering/render_form.moc \
	./kdesrc/khtml/rendering/render_frames.moc \
	./kdesrc/khtml/rendering/render_replaced.moc \
	./kdesrc/khtml/testkhtml.moc \
	./kdesrc/khtml/testrender.moc \
	./kdesrc/khtml/xml/dom_docimpl.moc \
	./kdesrc/khtml/xml/xml_tokenizer.moc \
	./kdesrc/kparts/browserextension.moc \
	./kdesrc/kparts/factory.moc \
	./kdesrc/kparts/historyprovider.moc \
	./kdesrc/kparts/partmanager.moc

CLEANFILES=$(moc_files)

makemoc: $(moc_files)
	@echo Checked moc dependencies.

%.moc: %.h
	echo moc $< -o $@
	@$(MOC) $< -o $@
