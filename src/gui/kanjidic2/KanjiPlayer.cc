/*
 *  Copyright (C) 2008/2009  Alexandre Courbot
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/TextTools.h"
#include "gui/kanjidic2/KanjiPlayer.h"

#include <QDebug>

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QToolButton>

#define TIMER_INTERVAL 20

#define STATE_STROKE 1
#define STATE_WAIT 2

#define CONTROL_ICON_SIZE 10

PreferenceItem<int> KanjiPlayer::animationSpeed("kanjidic", "animationSpeed", 30);
PreferenceItem<int> KanjiPlayer::delayBetweenStrokes("kanjidic", "delayBetweenStrokes", 10);

KanjiPlayer::KanjiPlayer(QWidget *parent) : QWidget(parent), _timer(), _kanji(0), renderer(), _picture(), _state(STATE_STROKE), _highlightedComponent(0)
{
	setAnimationSpeed(animationSpeed.value());
	setDelayBetweenStrokes(delayBetweenStrokes.value());

	kanjiView = new QLabel(this);
	kanjiView->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	kanjiView->setPicture(_picture);
	kanjiView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	kanjiView->setMouseTracking(true);
	kanjiView->installEventFilter(this);
	kanjiView->setAttribute(Qt::WA_Hover);

	strokeCountLabel = new QLabel(this);
	QFont font;
	font.setPointSize(font.pointSize() - 2);
	strokeCountLabel->setFont(font);
	strokeCountLabel->setAlignment(Qt::AlignHCenter);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(kanjiView, 0, Qt::AlignCenter);
	QHBoxLayout *controlLayout = new QHBoxLayout();
	controlLayout->setContentsMargins(0, 0, 0, 0);
	controlLayout->setSpacing(0);
	playButton = new QToolButton(this);
	playButton->setMaximumSize(20, 20);
	playButton->setIcon(QIcon(QPixmap(":/images/icons/control-play.png").scaledToHeight(CONTROL_ICON_SIZE, Qt::SmoothTransformation)));
	controlLayout->addWidget(playButton);
	resetButton = new QToolButton(this);
	resetButton->setMaximumSize(20, 20);
	resetButton->setIcon(QIcon(QPixmap(":/images/icons/control-stop.png").scaledToHeight(CONTROL_ICON_SIZE, Qt::SmoothTransformation)));
	controlLayout->addWidget(resetButton);
	controlLayout->addWidget(strokeCountLabel);
	prevButton = new QToolButton(this);
	prevButton->setMaximumSize(20, 20);
	prevButton->setIcon(QIcon(QPixmap(":/images/icons/control-fr.png").scaledToHeight(CONTROL_ICON_SIZE, Qt::SmoothTransformation)));
	controlLayout->addWidget(prevButton);
	nextButton = new QToolButton(this);
	nextButton->setMaximumSize(20, 20);
	nextButton->setIcon(QIcon(QPixmap(":/images/icons/control-ff.png").scaledToHeight(CONTROL_ICON_SIZE, Qt::SmoothTransformation)));
	controlLayout->addWidget(nextButton);
	mainLayout->addLayout(controlLayout);

	connect(playButton, SIGNAL(clicked()), this, SLOT(playButtonClicked()));
	connect(resetButton, SIGNAL(clicked()), this, SLOT(resetButtonClicked()));
	connect(prevButton, SIGNAL(clicked()), this, SLOT(prevStroke()));
	connect(nextButton, SIGNAL(clicked()), this, SLOT(nextStroke()));
	playButton->setEnabled(false);
	resetButton->setEnabled(false);
	prevButton->setEnabled(false);
	nextButton->setEnabled(false);

	QFontMetrics metrics(font);
	strokeCountLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	// Determine the largest size that our label can take.
	int maxWidth(0);
	for (char c = '0'; c <= '9';c++) maxWidth = qMax(metrics.width(c), maxWidth);
	strokeCountLabel->setMinimumWidth(metrics.width("/") + maxWidth * 4);

	setPictureSize(100);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	_timer.setInterval(TIMER_INTERVAL);
	connect(&_timer, SIGNAL(timeout()), this, SLOT(updateAnimationState()));
}

void KanjiPlayer::setKanji(const Kanjidic2Entry *kanji)
{
	_entry = kanji;
	_kanji = kanji;
	_highlightedComponent = 0;
	strokeCountLabel->setText("");
	playButton->setEnabled(true);
	renderer.setKanji(kanji);
	reset();
	renderCurrentState();
	updateStrokesCountLabel();
	update();
}

void KanjiPlayer::setPictureSize(int newSize)
{
	_pictureSize = newSize;
	_picture.setBoundingRect(QRect(0, 0, newSize, newSize));
	kanjiView->clear();
	kanjiView->setPicture(_picture);
	kanjiView->setMinimumSize(newSize, newSize);
}

void KanjiPlayer::updateButtonsState()
{
	// Update play button pixmap
	if (_timer.isActive()) playButton->setIcon(QIcon(QPixmap(":/images/icons/control-pause.png").scaledToHeight(CONTROL_ICON_SIZE, Qt::SmoothTransformation)));
	else playButton->setIcon(QIcon(QPixmap(":/images/icons/control-play.png").scaledToHeight(CONTROL_ICON_SIZE, Qt::SmoothTransformation)));
	resetButton->setEnabled(_strokesCpt < renderer.strokes().size());
	prevButton->setEnabled(_strokesCpt > 0);
	nextButton->setEnabled(_strokesCpt < renderer.strokes().size());
}

void KanjiPlayer::updateAnimationState()
{
	if (_state == STATE_WAIT) {
		if (++_lengthCpt >= _delayBetweenStrokes) {
			_lengthCpt = 0;
			_state = STATE_STROKE;
		}
	}
	else {
		if (_strokesCpt >= renderer.strokes().size()) {
			stop();
			return;
		}
		const KanjiRenderer::Stroke &currentStroke(renderer.strokes()[_strokesCpt]);
		bool updateLabel = _lengthCpt == 0;
		_lengthCpt += _animationSpeed;
		if (updateLabel) updateStrokesCountLabel();
		if (_lengthCpt > currentStroke.length()) {
			nextStroke();
			_state = STATE_WAIT;
		}
		update();
	}
}

void KanjiPlayer::highlightComponent(const KanjiComponent *component)
{
	if (component == _highlightedComponent) return;
	_highlightedComponent = component;
	// Only schedule a redraw if the animation is stopped
	if (!_timer.isActive()) update();
}

void KanjiPlayer::unHighlightComponent()
{
	highlightComponent(0);
}

void KanjiPlayer::renderCurrentState()
{
	QPen pen;
	pen.setWidth(4);
	pen.setColor(palette().color(QPalette::Dark));
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	QPen pen2;
	pen2.setWidth(4);
	pen2.setCapStyle(Qt::RoundCap);
	pen2.setJoinStyle(Qt::RoundJoin);

	QPainter painter(&_picture);
	painter.scale(pictureSize() / 109.0, pictureSize() / 109.0);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(pen);
	renderer.renderOutline(&painter);
	painter.setPen(pen2);

	const QList<const KanjiComponent *> &kComponents(_kanji->rootComponents());
	const QList<KanjiStroke> &kStrokes(_kanji->strokes());
	QList<QColor> colList;
	colList << Qt::black << Qt::darkBlue << Qt::darkRed << Qt::darkGreen << Qt::darkCyan << Qt::darkMagenta << Qt::darkYellow << Qt::blue << Qt::red << Qt::green << Qt::cyan << Qt::magenta << Qt::yellow;

	if (renderer.strokes().isEmpty()) return;
	// No highlighted component, render the animation
	if (!highlightedComponent()) {
		// Render full strokes
		for (int i = 0; i < _strokesCpt; i++) {
			const KanjiComponent *parent(kStrokes[i].parent());
			while (parent && !kComponents.contains(parent)) parent = parent->parent();
			if (!parent) pen2.setColor(colList[0]);
			else pen2.setColor(colList[kComponents.indexOf(parent) + 1]);
			painter.setPen(pen2);
			renderer.strokes()[i].render(&painter);
		}
		// Render partial stroke
		if (_state == STATE_STROKE && _strokesCpt < renderer.strokes().size()) {
			const KanjiRenderer::Stroke &currentStroke(renderer.strokes()[_strokesCpt]);
			const KanjiComponent *parent(currentStroke.stroke()->parent());
			while (parent && !kComponents.contains(parent)) parent = parent->parent();
			if (!parent) pen2.setColor(colList[0]);
			else pen2.setColor(colList[kComponents.indexOf(parent) + 1]);
			painter.setPen(pen2);
			currentStroke.render(&painter, _lengthCpt);
		}
	}
	// Render the highlighted component only
	else {
		pen2.setColor(colList[kComponents.indexOf(highlightedComponent()) + 1].lighter(200));
		pen2.setWidth(pen2.width() + 1);
		painter.setPen(pen2);
		renderer.renderComponentStrokes(*highlightedComponent(), &painter);
	}
	painter.end();
}

void KanjiPlayer::paintEvent(QPaintEvent * event)
{
	renderCurrentState();
	QWidget::paintEvent(event);
}

bool KanjiPlayer::eventFilter(QObject *obj, QEvent *event)
{
	if (obj != kanjiView) return false;
	if (event->type() == QEvent::MouseMove) {
		QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
		// First translate the coordinates to the strokes coordinates system
		QPointF pos(mEvent->posF());
		QPointF fPos(pos.x() * (109.0 / pictureSize()), pos.y() * (109.0 / pictureSize()));
		const QList<KanjiRenderer::Stroke> &strokes(renderer.strokes());
		QPainterPathStroker stroker;
		stroker.setWidth(20);
		const KanjiComponent *comp(0);
		foreach (const KanjiRenderer::Stroke &stroke, strokes) {
			QPainterPath nPath(stroker.createStroke(stroke.painterPath()));
			if (nPath.contains(fPos)) {
				const KanjiComponent *parent(stroke.stroke()->parent());
				// Do not consider the root component
				if (!parent->parent()) continue;
				while (parent->parent() != _kanji->root()) parent = parent->parent();
				comp = parent;
				break;
			}
		}
		if (highlightedComponent() != comp) {
			emit componentUnHighlighted(highlightedComponent());
			qobject_cast<QWidget *>(obj)->unsetCursor();
		}
		if (comp) {
			highlightComponent(comp);
			emit componentHighlighted(comp);
			qobject_cast<QWidget *>(obj)->setCursor(QCursor(Qt::PointingHandCursor));
		}
	}
	if (event->type() == QEvent::MouseButtonPress && _highlightedComponent) {
		QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
		if (mEvent->button() == Qt::LeftButton) {
			emit componentClicked(_highlightedComponent);
			qobject_cast<QWidget *>(obj)->unsetCursor();
		}
	}
	else if (event->type() == QEvent::HoverLeave) {
		if (highlightedComponent()) emit componentUnHighlighted(highlightedComponent());
		unHighlightComponent();
		qobject_cast<QWidget *>(obj)->unsetCursor();
	}
	return false;
}

void KanjiPlayer::setPosition(int strokeNbr)
{
	if (strokeNbr > renderer.strokes().size()) strokeNbr = renderer.strokes().size();
	_strokesCpt = strokeNbr;
	_lengthCpt = 0.0;
	_state = STATE_STROKE;
	updateButtonsState();
	updateStrokesCountLabel();
	update();
}

void KanjiPlayer::play()
{
	if (_timer.isActive()) return;
	_timer.start();
	updateButtonsState();
	emit animationStarted();
}

void KanjiPlayer::stop()
{
	if (!_timer.isActive()) return;
	_timer.stop();
	updateButtonsState();
	emit animationStopped();
}

void KanjiPlayer::reset()
{
	stop();
	setPosition(0);
	// Reset the picture
	QPainter painter;
	painter.begin(&_picture);
	painter.end();
	emit animationReset();
}

void KanjiPlayer::nextStroke()
{
	if (_strokesCpt >= renderer.strokes().size()) return;
	setPosition(_strokesCpt + 1);
}

void KanjiPlayer::prevStroke()
{
	if (_strokesCpt <= 0) return;
	setPosition(_strokesCpt - 1);
}

void KanjiPlayer::playButtonClicked()
{
	if (_timer.isActive()) stop();
	else {
		if (_strokesCpt >= renderer.strokes().size()) reset();
		play();
	}
}

void KanjiPlayer::resetButtonClicked()
{
	stop();
	setPosition(renderer.strokes().size());
}

void KanjiPlayer::updateStrokesCountLabel()
{
	strokeCountLabel->setText(QString("%1/%2").arg(_strokesCpt + (_lengthCpt ? 1 : 0)).arg(renderer.strokes().size()));
}
