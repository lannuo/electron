// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_COMMON_WEB_CONTENTS_DELEGATE_H_
#define ATOM_BROWSER_COMMON_WEB_CONTENTS_DELEGATE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "brightray/browser/inspectable_web_contents_delegate.h"
#include "brightray/browser/inspectable_web_contents_impl.h"
#include "brightray/browser/inspectable_web_contents_view_delegate.h"
#include "chrome/browser/devtools/devtools_file_system_indexer.h"
#include "content/public/browser/web_contents_delegate.h"
#include "electron/buildflags/buildflags.h"

#if defined(TOOLKIT_VIEWS) && !defined(OS_MACOSX)
#include "atom/browser/ui/autofill_popup.h"
#endif

namespace base {
class SequencedTaskRunner;
}

namespace atom {

class AtomBrowserContext;
class NativeWindow;
class WebDialogHelper;

#if BUILDFLAG(ENABLE_OSR)
class OffScreenWebContentsView;
#endif

class CommonWebContentsDelegate
    : public content::WebContentsDelegate,
      public brightray::InspectableWebContentsDelegate,
      public brightray::InspectableWebContentsViewDelegate {
 public:
  CommonWebContentsDelegate();
  ~CommonWebContentsDelegate() override;

  // Creates a InspectableWebContents object and takes onwership of
  // |web_contents|.
  void InitWithWebContents(content::WebContents* web_contents,
                           AtomBrowserContext* browser_context,
                           bool is_guest);

  // Set the window as owner window.
  void SetOwnerWindow(NativeWindow* owner_window);
  void SetOwnerWindow(content::WebContents* web_contents,
                      NativeWindow* owner_window);

  // Returns the WebContents managed by this delegate.
  content::WebContents* GetWebContents() const;

  // Returns the WebContents of devtools.
  content::WebContents* GetDevToolsWebContents() const;

  brightray::InspectableWebContents* managed_web_contents() const {
    return web_contents_.get();
  }

  NativeWindow* owner_window() const { return owner_window_.get(); }

  bool is_html_fullscreen() const { return html_fullscreen_; }

 protected:
#if BUILDFLAG(ENABLE_OSR)
  virtual OffScreenWebContentsView* GetOffScreenWebContentsView() const;
#endif

  // content::WebContentsDelegate:
  content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) override;
  bool CanOverscrollContent() const override;
  content::ColorChooser* OpenColorChooser(
      content::WebContents* web_contents,
      SkColor color,
      const std::vector<blink::mojom::ColorSuggestionPtr>& suggestions)
      override;
  void RunFileChooser(content::RenderFrameHost* render_frame_host,
                      const content::FileChooserParams& params) override;
  void EnumerateDirectory(content::WebContents* web_contents,
                          int request_id,
                          const base::FilePath& path) override;
  void EnterFullscreenModeForTab(
      content::WebContents* source,
      const GURL& origin,
      const blink::WebFullscreenOptions& options) override;
  void ExitFullscreenModeForTab(content::WebContents* source) override;
  bool IsFullscreenForTabOrPending(
      const content::WebContents* source) const override;
  blink::WebSecurityStyle GetSecurityStyle(
      content::WebContents* web_contents,
      content::SecurityStyleExplanations* explanations) override;
  void HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) override;

  // Autofill related events.
#if defined(TOOLKIT_VIEWS) && !defined(OS_MACOSX)
  void ShowAutofillPopup(content::RenderFrameHost* frame_host,
                         content::RenderFrameHost* embedder_frame_host,
                         bool offscreen,
                         const gfx::RectF& bounds,
                         const std::vector<base::string16>& values,
                         const std::vector<base::string16>& labels);
  void HideAutofillPopup();
#endif

  // brightray::InspectableWebContentsDelegate:
  void DevToolsSaveToFile(const std::string& url,
                          const std::string& content,
                          bool save_as) override;
  void DevToolsAppendToFile(const std::string& url,
                            const std::string& content) override;
  void DevToolsRequestFileSystems() override;
  void DevToolsAddFileSystem(const std::string& type,
                             const base::FilePath& file_system_path) override;
  void DevToolsRemoveFileSystem(
      const base::FilePath& file_system_path) override;
  void DevToolsIndexPath(int request_id,
                         const std::string& file_system_path,
                         const std::string& excluded_folders_message) override;
  void DevToolsStopIndexing(int request_id) override;
  void DevToolsSearchInPath(int request_id,
                            const std::string& file_system_path,
                            const std::string& query) override;

  // brightray::InspectableWebContentsViewDelegate:
#if defined(TOOLKIT_VIEWS) && !defined(OS_MACOSX)
  gfx::ImageSkia GetDevToolsWindowIcon() override;
#endif
#if defined(USE_X11)
  void GetDevToolsWindowWMClass(std::string* name,
                                std::string* class_name) override;
#endif

  // Destroy the managed InspectableWebContents object.
  void ResetManagedWebContents(bool async);

 private:
  // DevTools index event callbacks.
  void OnDevToolsIndexingWorkCalculated(int request_id,
                                        const std::string& file_system_path,
                                        int total_work);
  void OnDevToolsIndexingWorked(int request_id,
                                const std::string& file_system_path,
                                int worked);
  void OnDevToolsIndexingDone(int request_id,
                              const std::string& file_system_path);
  void OnDevToolsSearchCompleted(int request_id,
                                 const std::string& file_system_path,
                                 const std::vector<std::string>& file_paths);

  // Set fullscreen mode triggered by html api.
  void SetHtmlApiFullscreen(bool enter_fullscreen);

  // The window that this WebContents belongs to.
  base::WeakPtr<NativeWindow> owner_window_;

  bool offscreen_ = false;

  // Whether window is fullscreened by HTML5 api.
  bool html_fullscreen_ = false;

  // Whether window is fullscreened by window api.
  bool native_fullscreen_ = false;

  // UI related helper classes.
#if defined(TOOLKIT_VIEWS) && !defined(OS_MACOSX)
  std::unique_ptr<AutofillPopup> autofill_popup_;
#endif
  std::unique_ptr<WebDialogHelper> web_dialog_helper_;

  scoped_refptr<DevToolsFileSystemIndexer> devtools_file_system_indexer_;

  // Make sure BrowserContext is alwasys destroyed after WebContents.
  scoped_refptr<AtomBrowserContext> browser_context_;

  // The stored InspectableWebContents object.
  // Notice that web_contents_ must be placed after dialog_manager_, so we can
  // make sure web_contents_ is destroyed before dialog_manager_, otherwise a
  // crash would happen.
  std::unique_ptr<brightray::InspectableWebContents> web_contents_;

  // Maps url to file path, used by the file requests sent from devtools.
  typedef std::map<std::string, base::FilePath> PathsMap;
  PathsMap saved_files_;

  // Map id to index job, used for file system indexing requests from devtools.
  typedef std::
      map<int, scoped_refptr<DevToolsFileSystemIndexer::FileSystemIndexingJob>>
          DevToolsIndexingJobsMap;
  DevToolsIndexingJobsMap devtools_indexing_jobs_;

  scoped_refptr<base::SequencedTaskRunner> file_task_runner_;

  base::WeakPtrFactory<CommonWebContentsDelegate> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(CommonWebContentsDelegate);
};

}  // namespace atom

#endif  // ATOM_BROWSER_COMMON_WEB_CONTENTS_DELEGATE_H_