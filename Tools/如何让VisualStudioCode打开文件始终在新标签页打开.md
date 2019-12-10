`Visual Studio Code`有一个默认设定，单击一个右侧侧边栏的文件是预览模式，如果不输入任何任何文本就始终保持预览模式。

预览模式是打开一个新文件，然后再打开一个新文件，第二个就会占用第一个窗口。详细信息可以查看：https://code.visualstudio.com/docs/getstarted/userinterface#_preview-mode

>文件处于预览模式有个标识，就是标题栏的文件名称是斜体，

如何关闭？
```json
默认设置
  // Controls if opened editors show as preview. Preview editors are reused until they are kept (e.g. via double click or editing) and show up with an italic font style.
  "workbench.editor.enablePreview": true,
```
在设置文件里设置`workbench.editor.enablePreview` 为`false`即可。