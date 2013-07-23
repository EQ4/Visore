#include <vicorrectionwidget.h>
#include <ui_vicorrectionwidget.h>

#include <vineuralcorrector.h>

#include <vigeneralcorrectionwidget.h>
#include <vineuralstructurewidget.h>
#include <vineuralweightinitializerwidget.h>
#include <vineuraltrainerwidget.h>
#include <vineuralerrorfunctionwidget.h>
#include <vineuralactivationfunctionwidget.h>
#include <vineuraltargetproviderwidget.h>

ViCorrectionWidget::ViCorrectionWidget(QWidget *parent)
	: ViWidget(parent)
{
	mUi = new Ui::ViCorrectionWidget();
	mUi->setupUi(this);

	// Neural Corrector
	mCorrectors.append("Neural Corrector");
	QTabWidget *neuralTabWidget = new QTabWidget();
	addTab(neuralTabWidget, "General", new ViGeneralCorrectionWidget());
	addTab(neuralTabWidget, "Structure", new ViNeuralStructureWidget());
	addTab(neuralTabWidget, "Weights", new ViNeuralWeightInitializerWidget());
	addTab(neuralTabWidget, "Activation Function", new ViNeuralActivationFunctionWidget());
	addTab(neuralTabWidget, "Trainer", new ViNeuralTrainerWidget());
	addTab(neuralTabWidget, "Target Provider", new ViNeuralTargetProviderWidget());
	addTab(neuralTabWidget, "Error Functions", new ViNeuralErrorFunctionWidget());
	mUi->stackedWidget->addWidget(neuralTabWidget);
}

ViCorrectionWidget::~ViCorrectionWidget()
{
	clear();
	delete mUi;
}

void ViCorrectionWidget::clear()
{
	mCurrentCorrector = 0;
	mMode = ViCorrectionMode::Quick;
}

void ViCorrectionWidget::addTab(QTabWidget *tabWidget, const QString &text, QWidget *widget)
{
	widget->setContentsMargins(9, 9, 9, 9);
	tabWidget->addTab(widget, text);
}

void ViCorrectionWidget::changeCorrector(QString name)
{
	if(mCorrectors.contains(name))
	{
		mCurrentCorrector = mCorrectors.indexOf(name);
		mUi->stackedWidget->setCurrentIndex(mCurrentCorrector);
	}
	else
	{
		LOG("Invalid corrector selected.", QtCriticalMsg);
	}
	changeMode(mMode);
}

void ViCorrectionWidget::changeMode(ViCorrectionMode::Mode mode)
{
	mMode = mode;
	int count1 = mUi->stackedWidget->count(), count2;
	QTabWidget *tabWidget;
	ViCorrectionMode *modeWidget;
	for(int i = 0; i < count1; ++i)
	{
		tabWidget = dynamic_cast<QTabWidget*>(mUi->stackedWidget->widget(i));
		if(tabWidget != NULL)
		{
			count2 = tabWidget->count();
			for(int j = 0; j < count2; ++j)
			{
				modeWidget = dynamic_cast<ViCorrectionMode*>(tabWidget->widget(j));
				if(modeWidget != NULL)
				{
					modeWidget->setMode(mMode);
				}
			}
		}
	}
}

QStringList ViCorrectionWidget::correctors()
{
	return mCorrectors;
}

ViModifyProcessor* ViCorrectionWidget::corrector()
{
	QTabWidget *tabWidget = dynamic_cast<QTabWidget*>(mUi->stackedWidget->widget(mCurrentCorrector));
	if(tabWidget == NULL)
	{
		LOG("Invalid widget used to retrieve the corrector.", QtCriticalMsg);
	}
	else if(mCurrentCorrector == 0)
	{
		return neuralCorrector(tabWidget);
	}

	return NULL;
}

ViModifyProcessor* ViCorrectionWidget::neuralCorrector(QTabWidget *tabWidget)
{
	// Structure
	ViNeuralNetwork *network = NULL;
	ViNeuralStructureWidget *structureWidget = dynamic_cast<ViNeuralStructureWidget*>(tabWidget->widget(1));
	if(structureWidget == NULL)
	{
		LOG("Can't access the neural structure settings.", QtCriticalMsg);
	}
	else
	{
		network = structureWidget->network();
	}

	// Activation Function
	ViNeuralActivationFunctionWidget *activationWidget = dynamic_cast<ViNeuralActivationFunctionWidget*>(tabWidget->widget(3));
	if(activationWidget == NULL)
	{
		LOG("Can't access the neural activation function settings.", QtCriticalMsg);
	}
	else if(network != NULL)
	{
		network->setActivationFunction(activationWidget->activationFunction());
	}

	// Trainer
	ViTrainer *trainer = NULL;
	qreal learningRate = 0;
	ViNeuralTrainerWidget *trainerWidget = dynamic_cast<ViNeuralTrainerWidget*>(tabWidget->widget(4));
	if(trainerWidget == NULL)
	{
		LOG("Can't access the neural trainer settings.", QtCriticalMsg);
	}
	else
	{
		trainer = trainerWidget->trainer();
		learningRate = trainer->learningRate();
	}

	// Weight Initializer
	ViNeuralWeightInitializerWidget *weightWidget = dynamic_cast<ViNeuralWeightInitializerWidget*>(tabWidget->widget(2));
	if(weightWidget == NULL)
	{
		LOG("Can't access the neural weight intializer settings.", QtCriticalMsg);
	}
	else if(network != NULL)
	{
		ViWeightInitializer *initializer = weightWidget->weightInitializer();
		initializer->initialize(network, learningRate);
		delete initializer;
	}

	// Target Provider
	ViTargetProvider *provider = NULL;
	ViNeuralTargetProviderWidget *providerWidget = dynamic_cast<ViNeuralTargetProviderWidget*>(tabWidget->widget(5));
	if(providerWidget == NULL)
	{
		LOG("Can't access the neural target provider settings.", QtCriticalMsg);
	}
	else
	{
		provider = providerWidget->targetProvider();
	}

	// Error Functions
	ViNeuralErrorFunctionWidget *errorWidget = dynamic_cast<ViNeuralErrorFunctionWidget*>(tabWidget->widget(6));
	if(errorWidget == NULL)
	{
		LOG("Can't access the neural error function settings.", QtCriticalMsg);
	}
	else if(trainer != NULL)
	{
		trainer->addErrorFunctions(errorWidget->errorFunctions());
	}

	if(network == NULL || trainer == NULL || provider == NULL)
	{
		if(network != NULL) delete network;
		if(trainer != NULL) delete trainer;
		if(provider != NULL) delete provider;
	}
	else
	{
		ViNeuralCorrector *corrector = new ViNeuralCorrector(network, trainer, provider);

		// General
		ViGeneralCorrectionWidget *generalWidget = dynamic_cast<ViGeneralCorrectionWidget*>(tabWidget->widget(0));
		if(generalWidget == NULL)
		{
			LOG("Can't access the general settings.", QtCriticalMsg);
		}
		else
		{
			corrector->setChannelMode(generalWidget->channelMode());
			corrector->setProcessMode(generalWidget->processMode());
			corrector->setModifyMode(generalWidget->modifyMode());
			corrector->setNoiseDetector(generalWidget->noiseDetector());
		}

		return corrector;
	}

	return NULL;
}
