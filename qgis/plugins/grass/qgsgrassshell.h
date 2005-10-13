#include <qstring.h>
#include <qtextedit.h>
#include <qprocess.h>
#include <qmessagebox.h>
#include <qsocketnotifier.h>
#include <qcstring.h>

#include "qgsgrasstools.h"
#include "qgsgrassshellbase.h"
    
class QgsGrassShellText;

extern "C" {
#include <stdio.h>
}

/* QgsGrassShellBase is simple pseudo terminal implementing 
 * very limited subset of VT100. OTOH it parses GUI format of 
 * messages printed by GRASS modules and adds icons for 
 * WARNING and ERROR and it also resets progress bar if a module
 * uses G_percent() function.
 *
 * On UNIX pseudo terminal is represented by a pair of files 
 * called master and slave which are connected. 
 * On LINUX the files are called /dev/ptmx (master) 
 * and /dev/pts/<number> (slave). The application emulating terminal
 * is connected master and the program running in terminal
 * (usually shell like bash,tcsh,sh) is connected to slave.
 * 
 * There is no support for pseudo terminals in Qt. Everything 
 * must be written using relatively low level C functions 
 * 
 * Input (LINUX):
 * keyboard -> X -> Qt -> QgsGrassShellBase::keyPressEvent()
 * -> /dev/ptmx -> /dev/pts/<number> -> shell
 *
 * Output (LINUX):
 * shell -> /dev/pts/<number> -> /dev/ptmx 
 * -> QgsGrassShell::readStdout() -> QCString -> 
 * -> QgsGrassShell::printStdout() -> QTextEdit
 *
 * Reading/printing of shel output is bease on two prerrequisites:
 *   1) Lines printed by shell are not endless
 *   2) GRASS GUI messages are always available complete on output
 * thus we read everything on output to buffer in cycle untill 
 * we reach either EOF (no more data available) or NL (new line).
 * Then the data from buffer are printed to output QTextEdit.
 * Control characters, control sequences and GRASS GUI messages
 * are transformed to appropriate output written to QTextEdit.
 */

class QgsGrassShell: public QgsGrassShellBase
{
    Q_OBJECT;

public:
    QgsGrassShell ( QgsGrassTools *tools, 
                    QWidget * parent = 0, const char * name = 0 );
    ~QgsGrassShell();

    // Modes 
    enum Mode
    {
	Origin = 0,
	Wrap,
	Insert, 
	Screen,
	Cursor,
	NewLine,
	ModeCount
    };

    // Graphic Rendition
    enum Rendition
    {
        Bold = 0,
        Underline,
        Blink,
        Reverse,
        RendetionCount
    };

    // Pressed key 
    enum KeyDown
    {
	DownControl = 0,
	DownShift,
	DownAlt, 
	DownMeta,
	KeyDownCount
    };

    // Print maximum form standard input buffer
    void printStdout();

    // Print maximum form standard error buffer
    void printStderr();

    // Set / reset mode
    void setMode ( int m ) { mMode[m] = true; }
    void resetMode ( int m ) { mMode[m] = false; }

    // Insert text on current cursor position according to Insert mode
    void insert ( QString );

    // Start new line
    void newLine();

    // Erase cursor highlight (space)
    void eraseCursor(void);

    // Highlite cursor
    void showCursor(void);

    // Reset pseudoterminal size
    void resizeTerminal ();

    // Remove empty (or with whitespaces) paragraphs
    void removeEmptyParagraphs();

    // Reset cursor to current position
    //void setCursorPosition();

public slots:
    void readStdout(int socket);
    void readStderr();
    void keyPressEvent ( QKeyEvent * e );
    void keyReleaseEvent ( QKeyEvent * e );
    void mousePressEvent(QMouseEvent* e);

signals:

private:
    //QProcess *mProcess;
    // Master file descriptor
    int mFdMaster;

    /**
     *    * The socket descriptors for stdout.
     *       */
    int out[2];

    QSocketNotifier *mOutNotifier;

    // Buffer for data read form shell stdout
    QCString mStdoutBuffer; 

    // Buffer for data read form shell stderr
    QCString mStderrBuffer; 

    // Modes
    bool mMode[ModeCount];

    // Flag for space if it was added at the end of paragraph to 
    // visualize the cursor
    bool mCursorSpace; 

    // Current cursor position
    int mParagraph;
    int mIndex;

    // Text widget
    QgsGrassShellText *mText;

    // Default text font
    QFont mFont;

    // Rendetion
    bool mRendetion[RendetionCount];

    // Pressed keys
    bool mKeyDown[KeyDownCount];

    // Application directory
    QString mAppDir;

    // GRASS Tools
    QgsGrassTools *mTools;

    // Create new line in nex insert
    // This must be used because QTextEdit will remove all empty
    // paragraphs
    bool mNewLine;

    // Is it successfuly opened
    bool mValid;

    // Horizontal tabulator stop
    std::vector<bool> mTabStop;
};

class QgsGrassShellText : public QTextEdit
{
    Q_OBJECT;
public:
    QgsGrassShellText ( QgsGrassShell *, 
	                QWidget * parent = 0, const char * name = 0 );
    ~QgsGrassShellText();
public slots:
    void contentsMousePressEvent(QMouseEvent *);
    void resizeEvent ( QResizeEvent *);
    void keyPressEvent ( QKeyEvent * e );
    void keyReleaseEvent ( QKeyEvent * e );
private:
    QgsGrassShell *mShell;
};

