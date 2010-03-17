%% lofar.cls 
%%
%% Author: G.W. Kant
%% Date:   20 February 2002
%% Copyright (C) 2002, by CONCERO
%%
%% This class is freely donated by CONCERO to the LOFAR project 
%% organisation. It may be used and modified freely, however it is 
%% not allowed to remove the copyright message.
%%
%% History:
%% 
%% 2002-2-20 - 0.9.1  Created first version
%% 2002-2-21 - 0.9.2  Changed page header and incorporated the LOFAR logo
%% 2002-2-26 - 0.9.3  Solved section number bug

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   nerg.cls  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% ===========================================================================
%%%  @LaTeX-style-file{
%%%    author          = "Dion Kant",
%%%    version         = "0.9.3",
%%%    date            = "February 26th, 2002",
%%%    filename        = "lofar.cls",
%%%    address         = "CONCERO, Havelte, The Netherlands"
%%%    email           = "dion@concero.nl,
%%%    keywords        = "LaTeX, LOFAR, report",
%%%    mainserver      = "via E-mail from ??? (tbd) ",
%%%    docstring       = "This is a LaTeX style file for preparation
%%%                       of submission and final versions of articles
%%%                       to be published within the LOFAR project.
%%%                       The default font size is 11 points but 9pt,
%%%                       10pt and 12pt are available too.
%%%
%%%                        Usage:
%%%                         \documentstyle[..,Xpt,twoside]{nerg}
%%%                         \author{..}
%%%                         \title{..}
%%%                         \maketitle
%%%                         \begin{abstract}...\end{abstract}
%%%                         \begin{keywords}...\end{keywords}
%%%                         ...
%%%                         \begin{biography}{Author's name}...\end{biography}
%%%                         \end{document}
%%%
%%%  }
%%% ===========================================================================
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 
%% \CharacterTable
%%  {Upper-case    \A\B\C\D\E\F\G\H\I\J\K\L\M\N\O\P\Q\R\S\T\U\V\W\X\Y\Z
%%   Lower-case    \a\b\c\d\e\f\g\h\i\j\k\l\m\n\o\p\q\r\s\t\u\v\w\x\y\z
%%   Digits        \0\1\2\3\4\5\6\7\8\9
%%   Exclamation   \!     Double quote  \"     Hash (number) \#
%%   Dollar        \$     Percent       \%     Ampersand     \&
%%   Acute accent  \'     Left paren    \(     Right paren   \)
%%   Asterisk      \*     Plus          \+     Comma         \,
%%   Minus         \-     Point         \.     Solidus       \/
%%   Colon         \:     Semicolon     \;     Less than     \<
%%   Equals        \=     Greater than  \>     Question mark \?
%%   Commercial at \@     Left bracket  \[     Backslash     \\
%%   Right bracket \]     Circumflex    \^     Underscore    \_
%%   Grave accent  \`     Left brace    \{     Vertical bar  \|
%%   Right brace   \}     Tilde         \~}
\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{lofar}
              [2002/02/21 v0.9.3
(A class for LOFAR reports)]
\newcommand\@ptsize{}
\newif\if@restonecol
\newif\if@titlepage
\@titlepagefalse
\DeclareOption{a4paper}
   {\setlength\paperheight {297mm}%
    \setlength\paperwidth  {210mm}}
\DeclareOption{10pt}{\renewcommand\@ptsize{0}}
\DeclareOption{11pt}{\renewcommand\@ptsize{1}}
\DeclareOption{12pt}{\renewcommand\@ptsize{2}}
\DeclareOption{oneside}{\@twosidefalse \@mparswitchfalse}
\DeclareOption{twoside}{\@twosidetrue  \@mparswitchtrue}
\DeclareOption{draft}{\setlength\overfullrule{5pt}}
\DeclareOption{final}{\setlength\overfullrule{0pt}}
\DeclareOption{onecolumn}{\@twocolumnfalse}
\DeclareOption{twocolumn}{\@twocolumntrue}
\DeclareOption{leqno}{\input{leqno.clo}}
\DeclareOption{fleqn}{\input{fleqn.clo}}
\DeclareOption{openbib}{%
  \AtEndOfPackage{%
   \renewcommand\@openbib@code{%
      \advance\leftmargin\bibindent
      \itemindent -\bibindent
      \listparindent \itemindent
      \parsep \z@
      }%
   \renewcommand\newblock{\par}}%
}

%% define new needed boolean variables
\newif\if@draftversion  \@draftversionfalse
\newif\if@technote      \@technotefalse


\ExecuteOptions{a4paper,11pt,oneside,onecolumn,final}
\ProcessOptions
\input{size1\@ptsize.clo}
\setlength\lineskip{1\p@}
\setlength\normallineskip{1\p@}
\renewcommand\baselinestretch{}
\setlength\parskip{0\p@ \@plus \p@}
\@lowpenalty   51
\@medpenalty  151
\@highpenalty 301
\setcounter{topnumber}{2}
\renewcommand\topfraction{.7}
\setcounter{bottomnumber}{1}
\renewcommand\bottomfraction{.3}
\setcounter{totalnumber}{3}
\renewcommand\textfraction{.2}
\renewcommand\floatpagefraction{.5}
\setcounter{dbltopnumber}{2}
\renewcommand\dbltopfraction{.7}
\renewcommand\dblfloatpagefraction{.5}

\newcommand{\theAuthor}    {}
\newcommand{\theDOIssue}   {}
\newcommand{\theScope}     {}
\newcommand{\theVerifier}  {}
\newcommand{\theKOIssue}   {}
\newcommand{\theDocNR}     {}
\newcommand{\theResp}      {}
\newcommand{\theStatus}    {}
\newcommand{\theFile}      {}
\newcommand{\theAppr}      {}
\newcommand{\theRevNR}     {}
%
%
% Define the title command
%
\newcommand{\theTitle}   {}
\newcommand{\theProject} {}
\newcommand{\theTAuthor} {}
%
%
% Define the front page bottom info
%
\newcommand{\theVNameList} {}
\newcommand{\theVSigList}  {}
\newcommand{\theVDateList} {}
\newcommand{\theVRevList}  {}
\newcommand{\theAHeada} {}
\newcommand{\theAHeadb} {}
\newcommand{\theAHeadc} {}
\newcommand{\theAManagera} {}
\newcommand{\theAManagerb} {}
\newcommand{\theAManagerc} {}

%
%
% Define the commands for the first and second columns of the distribution list
%
\newcommand{\dGList} {}
\newcommand{\ddGList}{}
\newcommand{\dOList} {}
%
% Define the commands for the Document history
%
\newcommand{\revList}     {}
\newcommand{\dateList}    {}
\newcommand{\sectionList} {}
\newcommand{\pageList}    {}
\newcommand{\changeList}  {}


%
%
% These packages are necessary and also the array package
% is required by the tabularx package.
%
\usepackage{fancyhdr}
\usepackage{tabularx}
\usepackage{epsfig}


% Get rid of the default line in the header
\renewcommand{\headrulewidth}{0pt}


%
% 1pt = 0.351459803515mm
%
% Adapt these to your convenience

%% change sizes and margins
\topmargin      -1.14cm
\oddsidemargin  -0.74cm
\evensidemargin 0mm
\textheight     220mm
\textwidth      175mm
\columnsep      7.7mm
\parindent      0pt
\headsep        20pt
\headheight     2.3cm
\setlength{\parskip}{\baselineskip}
\setlength{\hoffset}{0mm}
\setlength{\voffset}{0mm}
\setlength{\footskip}{1.5cm}
\def\baselinestretch{1.08}

\if@draftversion 
\topmargin       -4.0mm
\oddsidemargin      0mm
\evensidemargin     0mm
\textheight       247mm
\textwidth      175.0mm
\fi

%% Definitions for LISTS
%%
%% Change aspect of lists with
%% 1) \itemindent,  label indentation  wrt to left list margin
%% 2) \leftmargini, the indentation of the whole list (on left, first level)
\itemindent        -1em
\leftmargini        2em
%\itemindent         2em  % Alternative values: sometimes used..
%\leftmargini        0em
\leftmarginii       1em
\leftmarginiii    1.5em
\leftmarginiv     1.5em
\leftmarginv      1.0em
\leftmarginvi     1.0em
\labelsep           5pt
\leftmargin\leftmargini
\labelwidth         \z@

\def\@listI{\leftmargin\leftmargini} \@listI
\def\@listi{\leftmargin\leftmargini  \topsep \z@ plus 1pt minus 1pt}
\def\@listii{\leftmargin\leftmarginii\labelwidth\leftmarginii
    \advance\labelwidth-\labelsep    \topsep \z@}
\def\@listiii{\leftmargin\leftmarginiii\labelwidth\leftmarginiii
    \advance\labelwidth-\labelsep    \topsep \z@}
\def\@listiv{\leftmargin\leftmarginiv\labelwidth\leftmarginiv
    \advance\labelwidth-\labelsep    \topsep \z@}
\def\@listv{\leftmargin\leftmarginv\labelwidth\leftmarginv
    \advance\labelwidth-\labelsep    \topsep \z@}
\def\@listvi{\leftmargin\leftmarginvi\labelwidth\leftmarginvi
    \advance\labelwidth-\labelsep    \topsep \z@}

\if@twoside
  \def\ps@headings{%
      \let\@evenhead\@empty\let\@oddhead\@empty{}%
      \def\@oddfoot{\hrulefill\footnotesize\raisebox{-.5ex}{\ \rightmark\ }%
            \hrulefill\makebox[0cm][r]{\psboxit{box .75 setgray fill}{%
            \spbox{\rule{6mm}{0mm}\raisebox{-.5ex}{\makebox[0mm]{\normalsize\the
      \def\@evenfoot{\psboxit{box .75 setgray fill}{\spbox{\rule{8mm}{0mm}%
            \raisebox{-.6ex}{\makebox[0mm]{\thepage}}\rule{6mm}{0mm}}}\hrulefill
\else
  \def\ps@headings{%
    \let\@oddfoot\@empty
    \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
    \let\@mkboth\markboth
    \def\sectionmark##1{%
      \markright {\MakeUppercase{%
        \ifnum \c@secnumdepth >\m@ne
          \thesection\quad
        \fi
        ##1}}}}
\fi
\def\ps@myheadings{%
    \let\@oddfoot\@empty\let\@evenfoot\@empty
    \def\@evenhead{\thepage\hfil\slshape\leftmark}%
    \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
    \let\@mkboth\@gobbletwo
    \let\sectionmark\@gobble
    \let\subsectionmark\@gobble
    }

\def\rightmark{}\def\leftmark{}

%% Defines the command for putting the header. footernote{TEXT} is the same
%% as markboth{TEXT}{TEXT}. Here for compatibility with other style files.
\def\markboth#1#2{\def\leftmark{#1}\def\rightmark{#2}}
\def\footernote#1{\markboth{#1}{#1}}

\newcommand\maketitle{%
\par
  \begingroup
    \renewcommand\thefootnote{\@fnsymbol\c@footnote}%
    \def\@makefnmark{\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%
    \long\def\@makefntext##1{\parindent 1em\noindent
            \hb@xt@1.8em{%
                \hss\@textsuperscript{\normalfont\@thefnmark}}##1}%
    \if@twocolumn
        \twocolumn[\@maketitle]%
    \else
      \newpage
      \global\@topnum\z@   % Prevents figures from going at top of page.
      \@maketitle
    \fi
    %\thispagestyle{plain}\@thanks
  \endgroup
  \setcounter{footnote}{0}%
  \global\let\thanks\relax
  \global\let\maketitle\relax
  \global\let\@maketitle\relax
  \global\let\@thanks\@empty
  \global\let\@author\@empty
  \global\let\@date\@empty
  \global\let\@title\@empty
  \global\let\title\relax
  \global\let\author\relax
  \global\let\date\relax
  \global\let\and\relax
}
\def\@maketitle{%
  \newpage
\vspace*{0.2cm}
\begin{center}

{\bf \Large \theTitle}

\vspace*{0.2cm}

{\large \theTAuthor}

\vfill

\renewcommand{\arraystretch}{1.15}%
    \begin{tabularx}{0.95\textwidth}{|>{\setlength{\hsize}{1.4\hsize}}X
                                     |>{\setlength{\hsize}{1.3\hsize}}X
                                     |>{\setlength{\hsize}{0.7\hsize}}X
                                     |>{\setlength{\hsize}{0.6\hsize}}X|}\hline
      \multicolumn{4}{|c|}{\textbf{Verified:}}\\\hline
      Name & Signature &  Date & Rev.nr. \\\hline
       \renewcommand{\arraystretch}{2.5}%
       \begin{tabular}[t]{l}
          \theVNameList
       \end{tabular} & 
       \renewcommand{\arraystretch}{2.5}%
       \begin{tabular}[t]{l}
          \theVSigList
       \end{tabular} & 
       \renewcommand{\arraystretch}{2.5}%
       \begin{tabular}[t]{l}
          \theVDateList
       \end{tabular} & 
       \renewcommand{\arraystretch}{2.5}%
       \begin{tabular}[t]{l}
          \theVRevList
       \end{tabular}\\\hline
    \end{tabularx}

    \begin{tabularx}{0.95\textwidth}{|X|X|X|}\hline 
      \multicolumn{3}{|c|}{\textbf{Accepted:}}\\\hline
      {\theAHeada} & {\theAHeadb} & {\theAHeadc}\\\hline
       \theAManagera      & \theAManagerb    & \theAManagerc\\
       & & \\
       ..........................................&
       ..........................................&
       ..........................................\\
%       ..................&.................. &.................. \\
      \small\emph{date:} & \emph{date:} & \emph{date:}\\\hline
    \end{tabularx}


\renewcommand{\arraystretch}{1.0}%
\begin{tabularx}{0.95\textwidth}{|X|}\hline
 \\
 \small
 \hfill\raisebox{1ex}{\copyright}ASTRON \the\year\hfill\mbox{}\\
 \hfill All rights are reserved. Reproduction in whole or in part is\hfill\mbox{}\\
 \hfill prohibited without the written consent of the copyright owner.\hfill\mbox{}\\
 \\\hline
\end{tabularx}
\end{center}

\cleardoublepage
}

\if@technote
   \setcounter{secnumdepth}{0}
\else
   \setcounter{secnumdepth}{3}
\fi
\newcounter {section}
\newcounter {subsection}[section]
\newcounter {subsubsection}[subsection]
\newcounter {paragraph}[subsubsection]
\newcounter {subparagraph}[paragraph]
\renewcommand \thesection     {\@arabic\c@section}
\renewcommand\thesubsection   {\thesection.\@arabic\c@subsection}
\renewcommand\thesubsubsection{\thesubsection.\@arabic\c@subsubsection}
\def\thesubsubsectiondis{\Alph{subsection}.\arabic{subsubsection}}
\renewcommand\theparagraph    {\thesubsubsection.\@arabic\c@paragraph}
\renewcommand\thesubparagraph {\theparagraph.\@arabic\c@subparagraph}
\newcommand\section{\@startsection {section}{1}{\z@}%
                                   {-3.5ex \@plus -1ex \@minus -.2ex}%
                                   {2.3ex \@plus.2ex}%
                                   {\normalfont\Large\bfseries}}
\newcommand\subsection{\@startsection{subsection}{2}{\z@}%
                                     {-3.25ex\@plus -1ex \@minus -.2ex}%
                                     {1.5ex \@plus .2ex}%
                                     {\normalfont\large\bfseries}}
\newcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}%
                                     {-3.25ex\@plus -1ex \@minus -.2ex}%
                                     {1.5ex \@plus .2ex}%
                                     {\normalfont\normalsize\bfseries}}
\newcommand\paragraph{\@startsection{paragraph}{4}{\z@}%
                                    {3.25ex \@plus1ex \@minus.2ex}%
                                    {-1em}%
                                    {\normalfont\normalsize\bfseries}}
\newcommand\subparagraph{\@startsection{subparagraph}{5}{\parindent}%
                                       {3.25ex \@plus1ex \@minus .2ex}%
                                       {-1em}%
                                      {\normalfont\normalsize\bfseries}}
\if@twocolumn
  \setlength\leftmargini  {2em}
\else
  \setlength\leftmargini  {2.5em}
\fi
\leftmargin  \leftmargini
\setlength\leftmarginii  {2.2em}
\setlength\leftmarginiii {1.87em}
\setlength\leftmarginiv  {1.7em}
\if@twocolumn
  \setlength\leftmarginv  {.5em}
  \setlength\leftmarginvi {.5em}
\else
  \setlength\leftmarginv  {1em}
  \setlength\leftmarginvi {1em}
\fi
\setlength  \labelsep  {.5em}
\setlength  \labelwidth{\leftmargini}
\addtolength\labelwidth{-\labelsep}
\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty
\renewcommand\theenumi{\@arabic\c@enumi}
\renewcommand\theenumii{\@alph\c@enumii}
\renewcommand\theenumiii{\@roman\c@enumiii}
\renewcommand\theenumiv{\@Alph\c@enumiv}
\newcommand\labelenumi{\theenumi.}
\newcommand\labelenumii{(\theenumii)}
\newcommand\labelenumiii{\theenumiii.}
\newcommand\labelenumiv{\theenumiv.}
\renewcommand\p@enumii{\theenumi}
\renewcommand\p@enumiii{\theenumi(\theenumii)}
\renewcommand\p@enumiv{\p@enumiii\theenumiii}
\newcommand\labelitemi{\textbullet}
\newcommand\labelitemii{\normalfont\bfseries \textendash}
\newcommand\labelitemiii{\textasteriskcentered}
\newcommand\labelitemiv{\textperiodcentered}
\def\labelitemi{$\scriptstyle\bullet$}
\def\labelitemii{\bf --}
\def\labelitemiii{$\ast$}
\def\labelitemiv{$\cdot$}
\newenvironment{description}
               {\list{}{\labelwidth\z@ \itemindent-\leftmargin
                        \let\makelabel\descriptionlabel}}
               {\endlist}
\newcommand*\descriptionlabel[1]{\hspace\labelsep
                                \normalfont\bfseries #1}
\if@titlepage
  \newenvironment{abstract}{%
      \titlepage
      \null\vfil
      \@beginparpenalty\@lowpenalty
      \begin{center}%
        \bfseries \abstractname
        \@endparpenalty\@M
      \end{center}}%
     {\par\vfil\null\endtitlepage}
\else
  \newenvironment{abstract}{%
      \if@twocolumn
        \section*{\abstractname}%
      \else
        \small
        \begin{center}%
          {\bfseries \abstractname\vspace{-.5em}\vspace{\z@}}%
        \end{center}%
        \quotation
      \fi}
      {\if@twocolumn\else\endquotation\fi}
\fi
\newenvironment{verse}
               {\let\\\@centercr
                \list{}{\itemsep      \z@
                        \itemindent   -1.5em%
                        \listparindent\itemindent
                        \rightmargin  \leftmargin
                        \advance\leftmargin 1.5em}%
                \item\relax}
               {\endlist}
\newenvironment{quotation}
               {\list{}{\listparindent 1.5em%
                        \itemindent    \listparindent
                        \rightmargin   \leftmargin
                        \parsep        \z@ \@plus\p@}%
                \item\relax}
               {\endlist}
\newenvironment{quote}
               {\list{}{\rightmargin\leftmargin}%
                \item\relax}
               {\endlist}
\if@compatibility
\newenvironment{titlepage}
    {%
      \if@twocolumn
        \@restonecoltrue\onecolumn
      \else
        \@restonecolfalse\newpage
      \fi
      \thispagestyle{empty}%
      \setcounter{page}\z@
    }%
    {\if@restonecol\twocolumn \else \newpage \fi
    }
\else
\newenvironment{titlepage}
    {%
      \if@twocolumn
        \@restonecoltrue\onecolumn
      \else
        \@restonecolfalse\newpage
      \fi
      \thispagestyle{empty}%
      \setcounter{page}\@ne
    }%
    {\if@restonecol\twocolumn \else \newpage \fi
     \if@twoside\else
        \setcounter{page}\@ne
     \fi
    }
\fi
\newcommand\appendix{\par
  \setcounter{section}{0}%
  \setcounter{subsection}{0}%
  \gdef\thesection{\@Alph\c@section}}
\setlength\arraycolsep{5\p@}
\setlength\tabcolsep{6\p@}
\setlength\arrayrulewidth{.4\p@}
\setlength\doublerulesep{2\p@}
\setlength\tabbingsep{\labelsep}
\skip\@mpfootins = \skip\footins
\setlength\fboxsep{3\p@}
\setlength\fboxrule{.4\p@}
\renewcommand \theequation {\@arabic\c@equation}
\newcounter{figure}
\renewcommand \thefigure {\@arabic\c@figure}
\def\fps@figure{tbp}
\def\ftype@figure{1}
\def\ext@figure{lof}
\def\fnum@figure{\small\figurename~\thefigure}
\newenvironment{figure}
               {\@float{figure}}
               {\end@float}
\newenvironment{figure*}
               {\@dblfloat{figure}}
               {\end@dblfloat}
\newcounter{table}
\renewcommand\thetable{\@arabic\c@table}
\def\fps@table{tbp}
\def\ftype@table{2}
\def\ext@table{lot}
\def\fnum@table{\small\tablename~\thetable}
\newenvironment{table}
               {\@float{table}}
               {\end@float}
\newenvironment{table*}
               {\@dblfloat{table}}
               {\end@dblfloat}
\newlength\abovecaptionskip
\newlength\belowcaptionskip
\setlength\abovecaptionskip{10\p@}
\setlength\belowcaptionskip{0\p@}
\long\def\@makecaption#1#2{%
  \vskip\abovecaptionskip
  \sbox\@tempboxa{#1: #2}%
  \ifdim \wd\@tempboxa >\hsize
    #1: #2\par
  \else
    \global \@minipagefalse
    \hb@xt@\hsize{\hfil\box\@tempboxa\hfil}%
  \fi
  \vskip\belowcaptionskip}
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand*\cal{\@fontswitch\relax\mathcal}
\DeclareRobustCommand*\mit{\@fontswitch\relax\mathnormal}
\newcommand\@pnumwidth{1.55em}
\newcommand\@tocrmarg{2.55em}
\newcommand\@dotsep{4.5}
\setcounter{tocdepth}{3}
\newcommand\tableofcontents{%
    \section*{\contentsname
        \@mkboth{%
           \MakeUppercase\contentsname}{\MakeUppercase\contentsname}}%
    \@starttoc{toc}%
    }
\newcommand*\l@part[2]{%
  \ifnum \c@tocdepth >-2\relax
    \addpenalty\@secpenalty
    \addvspace{2.25em \@plus\p@}%
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      {\leavevmode
       \large \bfseries #1\hfil \hb@xt@\@pnumwidth{\hss #2}}\par
       \nobreak
    \endgroup
  \fi}
\newcommand*\l@section[2]{%
  \ifnum \c@tocdepth >\z@
    \addpenalty\@secpenalty
    \addvspace{1.0em \@plus\p@}%
    \setlength\@tempdima{1.5em}%
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      \leavevmode \bfseries
      \advance\leftskip\@tempdima
      \hskip -\leftskip
      #1\nobreak\hfil \nobreak\hb@xt@\@pnumwidth{\hss #2}\par
    \endgroup
  \fi}
\newcommand*\l@subsection{\@dottedtocline{2}{1.5em}{2.3em}}
\newcommand*\l@subsubsection{\@dottedtocline{3}{3.8em}{3.2em}}
\newcommand*\l@paragraph{\@dottedtocline{4}{7.0em}{4.1em}}
\newcommand*\l@subparagraph{\@dottedtocline{5}{10em}{5em}}
\newcommand\listoffigures{%
    \section*{\listfigurename
      \@mkboth{\MakeUppercase\listfigurename}%
              {\MakeUppercase\listfigurename}}%
    \@starttoc{lof}%
    }
\newcommand*\l@figure{\@dottedtocline{1}{1.5em}{2.3em}}
\newcommand\listoftables{%
    \section*{\listtablename
      \@mkboth{%
          \MakeUppercase\listtablename}{\MakeUppercase\listtablename}}%
    \@starttoc{lot}%
    }
\let\l@table\l@figure
\newdimen\bibindent
\setlength\bibindent{1.5em}
\newenvironment{thebibliography}[1]
     {\section*{\refname
        \@mkboth{\MakeUppercase\refname}{\MakeUppercase\refname}}%
      \list{\@biblabel{\@arabic\c@enumiv}}%
           {\settowidth\labelwidth{\@biblabel{#1}}%
            \leftmargin\labelwidth
            \advance\leftmargin\labelsep
            \@openbib@code
            \usecounter{enumiv}%
            \let\p@enumiv\@empty
            \renewcommand\theenumiv{\@arabic\c@enumiv}}%
      \sloppy
      \clubpenalty4000
      \@clubpenalty \clubpenalty
      \widowpenalty4000%
      \sfcode`\.\@m}
     {\def\@noitemerr
       {\@latex@warning{Empty `thebibliography' environment}}%
      \endlist}
\newcommand\newblock{\hskip .11em\@plus.33em\@minus.07em}
\let\@openbib@code\@empty
\newenvironment{theindex}
               {\if@twocolumn
                  \@restonecolfalse
                \else
                  \@restonecoltrue
                \fi
                \columnseprule \z@
                \columnsep 35\p@
                \twocolumn[\section*{\indexname}]%
                \@mkboth{\MakeUppercase\indexname}%
                        {\MakeUppercase\indexname}%
                \thispagestyle{plain}\parindent\z@
                \parskip\z@ \@plus .3\p@\relax
                \let\item\@idxitem}
               {\if@restonecol\onecolumn\else\clearpage\fi}
\newcommand\@idxitem{\par\hangindent 40\p@}
\newcommand\subitem{\@idxitem \hspace*{20\p@}}
\newcommand\subsubitem{\@idxitem \hspace*{30\p@}}
\newcommand\indexspace{\par \vskip 10\p@ \@plus5\p@ \@minus3\p@\relax}
\renewcommand\footnoterule{%
  \kern-3\p@
  \hrule\@width.4\columnwidth
  \kern2.6\p@}
\newcommand\@makefntext[1]{%
    \parindent 1em%
    \noindent
    \hb@xt@1.8em{\hss\@makefnmark}#1}
\newcommand\contentsname{Contents}
\newcommand\listfigurename{List of Figures}
\newcommand\listtablename{List of Tables}
\newcommand\refname{References}
\newcommand\indexname{Index}
\newcommand\figurename{Figure}
\newcommand\tablename{Table}
\newcommand\partname{Part}
\newcommand\appendixname{Appendix}
\newcommand\abstractname{Abstract}
\def\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space\number\day, \number\year}
\if@twoside
\else
  \raggedbottom
\fi
\if@twocolumn
  \twocolumn
  \sloppy
  \flushbottom
\else
  \onecolumn
\fi
%% ENVIRONEMTS
\def\QED{\mbox{\rule[0pt]{1.5ex}{1.5ex}}}
\def\proof{\noindent\hspace{2em}{\it Proof: }}
\def\endproof{\hspace*{\fill}~\QED\par\endtrivlist\unskip}
\def\@begintheorem#1#2{\tmpitemindent\itemindent\topsep 0pt\rm\trivlist
    \item[\hskip \labelsep{\indent\it #1\ #2:}]\itemindent\tmpitemindent}
\def\@opargbegintheorem#1#2#3{\tmpitemindent\itemindent\topsep 0pt\rm \trivlist
    \item[\hskip\labelsep{\indent\it #1\ #2\ \rm(#3)}]\itemindent\tmpitemindent}
\def\@endtheorem{\endtrivlist\unskip}
%% set up default PAGESTYLE
\pagenumbering{arabic}
%%
\if@draftversion \def\baselinestretch{1.5}\fi
%%
%% Definition for Big letter at the beginning of a paragraph
%%
\def\PARstart#1#2{\begingroup\def\par{\endgraf\endgroup\lineskiplimit=0pt}
    \setbox2=\hbox{\uppercase{#2} }\newdimen\tmpht \tmpht \ht2
    \advance\tmpht by \baselineskip\font\hhuge=cmr10 at \tmpht
    \setbox1=\hbox{{\hhuge #1}}
    \count7=\tmpht \count8=\ht1\divide\count8 by 1000 \divide\count7 by\count8
    \tmpht=.001\tmpht\multiply\tmpht by \count7\font\hhuge=cmr10 at \tmpht
    \setbox1=\hbox{{\hhuge #1}} \noindent \hangindent1.05\wd1
    \hangafter=-2 {\hskip-\hangindent \lower1\ht1\hbox{\raise1.0\ht2\copy1}%
    \kern-0\wd1}\copy2\lineskiplimit=-1000pt}
%%
\if@technote\def\PARstart#1#2{#1#2}\fi     % if technical note, disable it
\if@draftversion\def\PARstart#1#2{#1#2}\fi % if draft, disable it
%%
%% Allows to enter BIOGRAPHY leaving place for picture (adapts to font size)
%%
\newenvironment{biography}[1]{%
\section*{CV}
\vspace{-1cm}
\unitlength 1mm\bigskip\bigskip\bigskip\parskip=0pt\par%
\rule{0pt}{38mm}\vspace{-38mm}\par%   garantees correct page breaking
\noindent\setbox0\hbox{#1}%   box containing the frame
%\ht0=37mm\count10=\ht0\divide\count10 by\baselineskip%  calculates lines
%\global\hangindent29mm\global\hangafter-\count10}%
  \begin{picture}(0,0)(0,0)%
     \put(130,-36){\box0}%
  \end{picture}%
\global\hangindent-40mm\global\hangafter-\count50}%
{\par\rm\normalsize\global\hangindent0mm}
%% Allows to enter BIOGRAPHY leaving place for picture (adapts to font size)
%%
\newenvironment{biography2}[1]{%
\footnotesize\unitlength 1mm\bigskip\bigskip\bigskip\parskip=0pt\par%
\rule{0pt}{39mm}\vspace{-39mm}\par%   garantees correct page breaking
\noindent\setbox0\hbox{\framebox(25,32){}}%   box containing the frame
\ht0=37mm\count10=\ht0\divide\count10 by\baselineskip%  calculates lines
\global\hangindent29mm\global\hangafter-\count10%
\hskip-28.5mm\setbox0\hbox to 28.5mm {\raise-30.5mm\box0\hss}%
\dp0=0mm\ht0=0mm\box0\noindent\bf#1\rm}{
\par\rm\normalsize\global\hangindent0mm}

% A Few PostScript definitions to use with \psboxit
% Call \PScommands once at the begining of your program, this will
% define : box roundedbox rectcartouche and cartouche. They are 4
% PostScript programs. Change the values before setlinewidth end
% setgray to customize your boxes
%
%     Ex : \psboxit{25 cartouche}{blah blah}
%          \psboxit{rectcartouche}{blah blah}
%
\long\def\PScommands{\special{! TeXDict begin
/box{%                  Processes the path of a rectangle.
%                       Needs : x0 y0 x1 y1.
newpath 2 copy moveto 3 copy pop exch lineto 4 copy pop pop
lineto 4 copy exch pop exch pop lineto closepath } bind def
%
%
/min{ 2 copy  gt { exch } if pop } bind def%
/max{ 2 copy  lt { exch } if pop } bind def%
/roundedbox{%           Processes the path of a rounded rectangle.
%                       Needs : x0 y0 x1 y1 radius.
%       The bounding box is augmented by +/- radius to allow easily to
%       frame several rounded boxes around the same Texture box. Ex:
%  \psboxit{4 copy 15 roundedbox 25 roundedbox} {\spbox{Some Text}}
%       draws two scaled boxes arond the same word. Delete the `radius
%       sub' and `radius add' commands to suppress that enlargement.
%
/radius exch store
3 2 roll %              x0 x1 y1 y0
2 copy min radius sub /miny exch store 
       max radius add /maxy exch store
2 copy min radius sub /minx exch store 
       max radius add /maxx exch store
newpath
minx radius add miny moveto
maxx miny maxx maxy radius arcto
maxx maxy minx maxy radius arcto
minx maxy minx miny radius arcto
minx miny maxx miny radius arcto 16 {pop} repeat
closepath
}bind def
%
%
/rectcartouche{%        Draws a filled and framed box
%                       Needs : x0 y0 x1 y1
box gsave  .95 setgray fill grestore 1 setlinewidth stroke }bind def
%
%
/cartouche{%            Draws a filled and framed rounded box
%                       Needs : x0 y0 x1 y1 radius
roundedbox gsave .95 setgray fill grestore 1 setlinewidth stroke }bind def
%
end }%                  Closes dictionnary
}\PScommands%
%%
%% PSBOXIT
%%
%% \psboxit{PS program}{TeX stuff}
%%
%% The bounding box of the TeX stuff is pushed on the PostScript stack
%% and then the program in the first argument is called
%%
%% EXAMPLE: set some text on a gray background, Use the SPBOX macro to
%% give some space around the text.
%%
%%      \psboxit{box 0.5 setgray fill}{\spbox{Some Text}}
%%
%% See \PScommands for the \box definition
%%
\chardef \atcode = \the \catcode `\@
\catcode `\@ = 11
 
\long\def\psboxit#1#2{%
\begingroup\setbox0=\hbox{#2}%
\dimen0=\ht0 \advance\dimen0 by \dp0%
    % Write out the PS code to set the current path using HEIGHT,
    % WIDTH , DEPTH of box0.
    \hbox{%
    \special{ps: gsave currentpoint translate
        0
        \number\dp0 \space 15800 300 VResolution div mul DVImag div div    % han
        \number\wd0 \space 15800 300 Resolution div mul DVImag div div     % han
        \number\ht0 \space -15800 300 VResolution div mul DVImag div div  % hand
%        \number\dp0 \space 16384 div
%        \number\wd0 \space 16384 div
%        \number\ht0 \space -16384 div   % Bounding box
%        \number\dp0 \space 65536 div
%        \number\wd0 \space 65536 div
%        \number\ht0 \space -65536 div   % Bounding box
        #1 grestore}%
    \copy0%
}%HBOX
\endgroup%
}%
 
% SPACEBOX
%
% This macro simply takes some TeX stuff, and puts FOUR sides on it
% so that the box is the same size as the thing you'd get with
% an \fbox{} command.  (All I did was modify the code for \fbox{}
% so that all rules were replaced with struts).
%
% USAGE: \spbox{text} is just like \fbox{text} but makes no rules
%
% REASON: so that if using \pspath{...}{\fbox{stuff}}
%         there is a way to get another box the same size:
%         \pspath{...}{\spbox{stuff}}
%
\long\def\spbox#1{\leavevmode\setbox1\hbox{#1}%
    \dimen0\fboxrule \advance\dimen0 \fboxsep%
    \advance\dimen0 \dp1%
    \hbox{\lower \dimen0\hbox%
    {\vbox{\hrule height \fboxrule width 0pt%
          \hbox{\vrule width \fboxrule height 0pt \hskip\fboxsep%
          \vbox{\vskip\fboxsep \box1\vskip\fboxsep}\hskip%
                 \fboxsep\vrule width \fboxrule height 0pt}%
                 \hrule height \fboxrule width 0pt}}}}%
 
\def\Gbox#1{\psboxit{box 0.9 setgray fill}{#1}}
%%
\newsavebox{\gray@box}%
\newdimen\gray@space
%
\long\def\Beginboxitpara#1{\edef\gray@tmp{#1}%
\gray@space=\fboxsep \advance\gray@space by -\fboxrule
\par\prevdepth=-1000pt\vskip 0.5\baselineskip\noindent
\setbox\gray@box=%
\hbox\bgroup \vrule width \fboxrule
        \vtop\bgroup \hrule
             \vbox\bgroup
                   \kern\gray@space
                   \hbox\bgroup
                        \kern\gray@space
                        \vtop\bgroup
                   \hsize=\linewidth\advance\hsize-2\fboxsep\noindent
                   \ignorespaces}
\def\Endboxitpara{%
                             \egroup
                        \kern\gray@space
                        \egroup
                   \kern\gray@space
                   \egroup \hrule
             \egroup\vrule width \fboxrule
     \egroup
     \psboxit{\gray@tmp}{\box\gray@box}%
     \vskip 0.5\baselineskip
\par}

\newenvironment{boxitpara}[1]{\Beginboxitpara{#1}}{\Endboxitpara}
\newenvironment{boxitpara*}[1]{\Beginboxitpara{#1}\hbox to\hsize{}}%
{\Endboxitpara}

\catcode `\@ = \the \atcode

\def\@normalsize{\@setsize\normalsize{15pt}\xpt\@xpt
\abovedisplayskip 1em plus2pt minus5pt\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus3pt\belowdisplayshortskip .6em plus3pt minus3pt
\topsep \belowdisplayshortskip%!PN
}

\pagestyle{fancy}

\fancyhead{}
\fancyhead[LE,LO]{%
\renewcommand{\arraystretch}{1.2}%
\fontencoding{T1}\fontfamily{ptm}\fontseries{m}\fontshape{n}%
  \fontsize{7}{8}\selectfont
\begin{tabular}%
{|@{\hspace{.7mm}}p{0.8cm}@{}p{2.65cm}%
 |@{\hspace{.7mm}}p{1.52cm}@{}p{1.43cm}%
 |@{\hspace{.7mm}}p{0.9cm}@{}p{6.75cm}%
 |p{2.05cm}|} \hline%
% 
% so we are loosing 0.22cm per column.
  Author:& \theAuthor      &Date of issue:& \theDOIssue & Scope:   & \theScope&  \\[-0.25mm]
         &                 &Kind of issue:& \theKOIssue & Doc.nr.: & \theDocNR&
    \begin{picture}(0,0)(0,0)\put(30,-48){\makebox[0cm]{\epsfig{file=llofar.eps, width=1.92cm}}}\end{picture}
\\[4.75mm]\cline{1-6}
         &                 &Status:       & \theStatus  & File:    & \theFile  &  \\[-0.25mm]
         &                 &Revision nr.: & \theRevNR   &          &          &  \\[4.75mm]\hline
\end{tabular}% Removing this comment sign adds space behind the table!
}
\fancyfoot{}
\fancyfoot[LE]{\epsfig{file=astronple.eps}\\[-9mm]
{\scriptsize \mbox{}\hspace{12.5cm}\raisebox{1ex}{\copyright}ASTRON \the\year\\[-1mm]
             -{\thepage}-\hspace{1cm}\theProject--\theKOIssue}}
\fancyfoot[RO]{\epsfig{file=astronpro.eps}\\[-9mm]{
\scriptsize \raisebox{1ex}{\copyright}ASTRON \the\year\hspace{11.65cm}\mbox{}\\[-0.5mm]
 \theProject\hspace{1cm}-{\thepage}-}}

\newcommand{\theDistributionList} {%
%\cleardoublepage
%\addcontentsline{toc}{section}{Distribution List}
{\bf \Large Distribution list:}

\renewcommand{\arraystretch}{1.15}%
\vspace*{-1.5\baselineskip}
\rule{\linewidth}{0.5mm}
\begin{tabularx}{\linewidth}[t]{|X|X|}\hline
 {\bf Group:} & {\bf For Information:} \\ \hline
   \begin{tabular}[t]{@{}l}
      \dGList 
   \end{tabular}\begin{tabular}[t]{@{\hspace*{1cm}}l}
                    \ddGList 
                \end{tabular} & \begin{tabular}[t]{@{}l}
                      \dOList
                   \end{tabular}\\ \hline
\end{tabularx}

%\vspace{\fill}
%\mbox{}
}

\newcommand{\theDocumentRevision}{%
%\cleardoublepage
%\addcontentsline{toc}{section}{Document Revision}
{\bf \Large Document revision:}

\vspace*{-1.5\baselineskip}
\rule{\linewidth}{0.5mm}

\begin{tabular}{|c|l|c|c|l|}\hline
\makebox[1.7cm]{{\bf \hfill Revision\hfill}} & 
   \makebox[2.0cm]{{\bf Date\hfill}} & 
      \makebox[1.5cm]{{\bf \hfill Section\hfill}} & 
         \makebox[1.5cm]{{\bf \hfill Page(s)\hfill}} &
            \makebox[8.35cm]{{\bf Modification\hfill}}\\ \hline
\begin{tabular}[t]{@{}c}
   \revList
\end{tabular} &
   \begin{tabular}[t]{@{}l}
      \dateList
   \end{tabular} &
      \begin{tabular}[t]{@{}c}
         \sectionList
      \end{tabular} &
         \begin{tabular}[t]{@{}c}
            \pageList
         \end{tabular} &
            \begin{tabular}[t]{@{}l}
               \changeList
            \end{tabular} \\ \hline
\end{tabular}}%

\endinput

%%
%% End of file `lofar.cls'.
