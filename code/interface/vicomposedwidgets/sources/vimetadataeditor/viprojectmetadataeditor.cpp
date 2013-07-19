#include <viprojectmetadataeditor.h>
#include <ui_viprojectmetadataeditor.h>

ViProjectMetadataEditor::ViProjectMetadataEditor(QWidget *parent)
	: ViWidget(parent)
{
	mUi = new Ui::ViProjectMetadataEditor();
	mUi->setupUi(this);

	QFont font = mUi->artistButton->font();
	font.setPointSize(12);
	mUi->artistButton->setFont(font);
	mUi->albumButton->setFont(font);
	mUi->saveButton->setIcon(ViThemeManager::icon("save"), 36);

	QObject::connect(mUi->trackEditor, SIGNAL(changed()), this, SLOT(updateTracks()));
	QObject::connect(mUi->projectEdit, SIGNAL(textEdited(QString)), this, SLOT(changeProjectName()));
	QObject::connect(mUi->artistButton, SIGNAL(clicked()), this, SLOT(setGlobalArtist()));
	QObject::connect(mUi->albumButton, SIGNAL(clicked()), this, SLOT(setGlobalAlbum()));
	QObject::connect(mUi->saveButton, SIGNAL(clicked()), this, SLOT(save()));

	setStyleSheet(styleSheet() + "QLabel { width: 150px; min-width: 150px; }");

	mProject = NULL;
	clear();
}

ViProjectMetadataEditor::~ViProjectMetadataEditor()
{
	clear();
	delete mUi;
}

void ViProjectMetadataEditor::clear()
{
	QObject::disconnect(mUi->trackComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTrack())); // Important
	if(mTakeOwnership && mProject != NULL)
	{
		delete mProject;
	}
	mProject = NULL;

	mUi->trackEditor->hide();

	mTakeOwnership = false;
	mGlobalArtist = "";
	mGlobalAlbum = "";

	mUi->trackEditor->clear();
	mUi->projectEdit->clear();
	mUi->artistEdit->clear();
	mUi->albumEdit->clear();
	mUi->trackComboBox->clear();
}

void ViProjectMetadataEditor::setProject(ViProject *project, bool takeOwnership)
{
	clear();
	mTakeOwnership = takeOwnership;
	mProject = project;

	mUi->trackEditor->show();
	mUi->projectEdit->setText(mProject->projectName());
	updateTracks();
	changeTrack();

	QString artist = "";
	bool same = true;
	for(int i = 0; i < mProject->objectCount(); ++i)
	{
		if(artist == "")
		{
			artist = mProject->object(i)->metadata().artist();
		}
		else if(artist != mProject->object(i)->metadata().artist())
		{
			same = false;
			break;
		}
	}
	if(artist == ViMetadata::unknownArtist()) artist = "";
	if(same) mUi->artistEdit->setText(artist);
	else mUi->artistEdit->setText("Various Artists");
}

bool ViProjectMetadataEditor::hasProject()
{
	return mProject != NULL;
}

void ViProjectMetadataEditor::save()
{
	if(hasProject())
	{
		engine()->updateMetadata(mProject);
	}
}

void ViProjectMetadataEditor::changeTrack()
{
	int index = mUi->trackComboBox->currentIndex();
	if(index >= 0 && index < mProject->objectCount())
	{
		mUi->trackEditor->setMetadata(&mProject->object(index)->metadata());
		if(mGlobalArtist != "") mUi->trackEditor->setArtist(mGlobalArtist);
		if(mGlobalAlbum != "") mUi->trackEditor->setAlbum(mGlobalAlbum);
	}
}

void ViProjectMetadataEditor::updateTracks()
{
	QObject::disconnect(mUi->trackComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTrack()));
	if(hasProject())
	{
		int currentIndex = mUi->trackComboBox->currentIndex();
		mUi->trackComboBox->clear();
		for(int i = 0; i < mProject->objectCount(); ++i)
		{
			mUi->trackComboBox->addItem(mProject->object(i)->fileName(true, true));
			mUi->trackEditor->addPossibleImage(mProject->object(i)->metadata().cover());
		}
		if(currentIndex >= 0 && currentIndex < mUi->trackComboBox->count())
		{
			mUi->trackComboBox->setCurrentIndex(currentIndex);
		}
		else
		{
			mUi->trackComboBox->setCurrentIndex(0);
		}
	}
	QObject::connect(mUi->trackComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTrack()));
}

void ViProjectMetadataEditor::changeProjectName()
{
	if(hasProject())
	{
		mProject->setProjectName(mUi->projectEdit->text());
	}
}

void ViProjectMetadataEditor::setGlobalArtist()
{
	mGlobalArtist = mUi->artistEdit->text();
	if(mGlobalArtist != "") mUi->trackEditor->setArtist(mGlobalArtist);
}

void ViProjectMetadataEditor::setGlobalAlbum()
{
	mGlobalAlbum = mUi->albumEdit->text();
	if(mGlobalAlbum != "") mUi->trackEditor->setAlbum(mGlobalAlbum);
}