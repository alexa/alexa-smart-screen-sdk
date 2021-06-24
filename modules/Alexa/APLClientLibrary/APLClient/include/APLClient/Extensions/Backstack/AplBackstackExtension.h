/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_BACKSTACK_APLBACKSTACKEXTENSION_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_BACKSTACK_APLBACKSTACKEXTENSION_H

#include "APLClient/Extensions/AplCoreExtensionInterface.h"
#include "AplBackstackExtensionObserver.h"

namespace APLClient {
namespace Extensions {
namespace Backstack {

static const std::string URI = "aplext:backstack:10";
static const std::string PROPERTY_BACK_TYPE_INDEX = "index";
static const std::string PROPERTY_BACK_TYPE_ID = "id";

/**
 * The APL Backstack extension is an optional-use feature available for APL clients which allows
 * APL developers to provide users the ability to navigate back to previously viewed documents using
 * common device inputs and APL extension commands.
 *
 * https://developer.amazon.com/docs/alexa/alexa-presentation-language/apl-ext-backstack.html
 */
class AplBackstackExtension
        : public AplCoreExtensionInterface
        , public std::enable_shared_from_this<AplBackstackExtension> {
public:
    /**
     * Constructor
     */
    AplBackstackExtension(std::shared_ptr<AplBackstackExtensionObserverInterface> observer);

    ~AplBackstackExtension() override = default;

    /// @name AplCoreExtensionInterface Functions
    /// @{
    std::string getUri() override;

    apl::Object getEnvironment() override;

    std::list<apl::ExtensionCommandDefinition> getCommandDefinitions() override;

    std::list<apl::ExtensionEventHandler> getEventHandlers() override;

    std::unordered_map<std::string, apl::LiveObjectPtr> getLiveDataObjects() override;

    void applySettings(const apl::Object& settings) override;
    /// @}

    /// @name AplCoreExtensionEventCallbackInterface Functions
    /// @{
    void onExtensionEvent(
        const std::string& uri,
        const std::string& name,
        const apl::Object& source,
        const apl::Object& params,
        unsigned int event,
        std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback = nullptr) override;
    /// @}

    /**
     * Tells the @c AplBackstack if it should @c handleBack as invoked by a system event,
     * or if the document is responsible.
     *
     * Example:
     * A device that allows the AplClient and Apl documents to use the AplBackstack extension, but does not allow
     * any invocation of the backstack from it's system inputs (physical button or otherwise) would set this property
     * to be TRUE.
     *
     * @param isResponsibleForBackButton True if the device does not allow, or has no mechanism for, system invocation
     * of back (making the APL document "responsible").
     */
    void setResponsibleForBackButton(bool isResponsibleForBackButton);

    /**
     * @return True if there is an active document id to use for caching @c AplDocumentState.
     */
    bool shouldCacheActiveDocument();

    /**
     * Add the provided @c AplDocumentState to the @c AplBackstack
     * @param documentState The @c AplDocumentState to add to the backstack.
     */
    void addDocumentStateToBackstack(const AplDocumentStatePtr& documentState);

    /**
     * Clear the @c AplBackstack, and clear the active document id.
     */
    void reset();

    /**
     * Attempt to handle a system-invoked back event.
     * Gated by the value provided with @c setResponsibleForBackButton.
     * @return True if the extension allows system back and the back event succeeded in issuing a restoreDocumentState
     * callback to the observer.
     */
    bool handleBack();

private:
    /**
     * Enumerated back types
     */
    enum class AplBackType { COUNT, INDEX, ID };

    /**
     * Convert string to @c AplBackType
     * @param type The string value of the back type.
     * @return @c AplBackType (Defaults to COUNT).
     */
    static inline AplBackType backTypeFromString(const std::string& type) {
        AplBackType backType = AplBackType::COUNT;
        if (type == PROPERTY_BACK_TYPE_INDEX) {
            backType = AplBackType::INDEX;
        } else if (type == PROPERTY_BACK_TYPE_ID) {
            backType = AplBackType::ID;
        }
        return backType;
    }

    /**
     * Contains @c AplDocumentState objects and methods to support the APL Backstack.
     *
     * Note:
     * The top of the stack is {@c AplBackstack.length()} - 1.
     */
    struct AplBackstack {
        /**
         * Default Constructor.
         */
        AplBackstack() = default;

        std::string id;
        std::vector<std::shared_ptr<AplDocumentState>> m_documentStateCache;

        /// The @c apl::LiveArray data for the backstack id's.
        apl::LiveArrayPtr m_backstackIds = apl::LiveArray::create();

        /**
         * Adds a document to the Backstack.
         * @param documentState the @c AplDocumentState to add.
         */
        void addDocumentState(const AplDocumentStatePtr& documentState) {
            m_documentStateCache.emplace_back(documentState);
            m_backstackIds->push_back(documentState->id);
        }

        /**
         * @return the length of the Backstack.
         */
        unsigned int length() const {
            return m_documentStateCache.size();
        }

        /**
         * @return the @c LiveArray of document ids in the backstack.
         */
        apl::LiveArrayPtr getBackstackIds() const {
            return m_backstackIds;
        }

        /**
         * @return the list of document ids in the backstack.
         */
        apl::ObjectArray getBackstackIdsArray() const {
            apl::ObjectArray backstackIds;
            for (const auto & it : m_backstackIds->getArray()) {
                backstackIds.emplace_back(it);
            }
            return backstackIds;
        }

        /**
         * Gets the index of the most recent document with the id backstackId.
         *
         * Note: documents are stored in ascending-recency order. That is, the order ['A','B','C'] means
         * that 'C' is the most recent document.
         *
         * @param backstackId the id to search for.
         * @return the index of the most recent document in the stack matching backstackId,
         *      or -1 if not found.
         */
        int indexOf(const std::string& backstackId) {
            auto index = length() - 1;

            for (auto rit = m_documentStateCache.rbegin(); rit != m_documentStateCache.rend(); rit++) {
                if (backstackId == rit->get()->id) {
                    break;
                }
                index--;
            }

            return index;
        }

        /**
         * Removes all documents in the stack more recent than the most recent document with matching
         * backstackId and removes and returns that document.
         *
         * For example, if the stack is ['A','B','B','C'], then popDocuments('B') would return the document
         * at index 2 and the stack would be ['A','B'].
         *
         * @param backstackId the id of the document to return
         * @return the most recent document in the stack whose document id matches the parameter.
         */
        AplDocumentStatePtr popDocuments(const std::string& backstackId) {
            auto indexOfDocument = indexOf(backstackId);
            if (indexOfDocument == -1) {
                return nullptr;
            }
            return popDocumentsAtIndex(indexOfDocument);
        }

        /**
         * Removes count documents from the stack and returns the last one removed.
         *
         * For example, if the stack has documents ['A', 'B', 'C'], then popDocuments(2) would return 'B',
         * and the stack would be: ['A'].
         *
         * @param count the number of documents to remove
         * @return the count document in the stack
         */
        AplDocumentStatePtr popDocuments(unsigned int count) {
            if (count < 0 || count > length()) {
                return nullptr;
            }

            m_documentStateCache.erase(m_documentStateCache.end() - (count - 1), m_documentStateCache.end());
            m_backstackIds->remove(m_backstackIds->size() - count, count);
            auto documentState = m_documentStateCache.back();
            m_documentStateCache.pop_back();

            return documentState;
        }

        /**
         * Removes all documents more recent than and including index and returns the document at index.
         *
         * For example, if the stack has ['A','B','C'], then both popDocumentsAtIndex(0) and
         * popDocumentsAtIndex(-3) would return 'A' and the stack would be [].
         *
         * @param index the index of the document to return (can be negative to count backwards)
         * @return the document at index.
         */
        AplDocumentStatePtr popDocumentsAtIndex(unsigned int index) {
            // Convert negative indexes to positive
            if (index < 0) {
                index = index + length();
            }

            if (index < 0 || index >= length()) {
                return nullptr;
            }

            return popDocuments(length() - index);
        }

        /**
         * Clears the backstack of all @c AplDocumentState's
         */
        void clear() {
            m_documentStateCache.clear();
            m_backstackIds->clear();
        }
    };

    /**
     * Handle the GoBack Extension Command
     * @param params The command params.
     * @return True if the command was successful.
     */
    bool handleGoBack(const apl::Object& params);

    /**
     * Go Back by Count.
     * @param count The number of documents in the stack by which to go back.
     * @return True if the command was successful.
     */
    bool goBackCount(unsigned int count);

    /**
     * Go Back by Index.
     * @param index The index of the document to return to in the stack.
     * @return True if the command was successful.
     */
    bool goBackIndex(unsigned int index);

    /**
     * Go Back by Id.
     * @param id The id of the document to return to in the stack.
     * @return True if the command was successful.
     */
    bool goBackId(const std::string& id);

    /**
     * Notify the observer to restore the @c AplDocumentState popped by the backstack.
     * @param documentState The @c AplDocumentState to restore.
     * @return True if @c AplDocumentState can be restored and the observer was notified.
     */
    bool restoreDocumentState(const AplDocumentStatePtr& documentState);

    /**
     * Clear the active document id tracked by the extension.
     */
    void clearActiveDocumentId();

    /// Indicates if the extension allows external back input,
    /// or if the document is responsible for drawing a back button.
    bool m_responsibleForBackButton;

    /// The active backstack id as provided by the last requesting document in settings.
    std::string m_activeDocumentId;

    /// The document settings defined 'name' for the backstack array data object.
    std::string m_backstackArrayName;

    /// The @c AplBackstack used for tracking @c AplDocumentStates.
    AplBackstack m_backstack;

    /// The @c AplBackstackExtensionObserverInterface observer.
    std::shared_ptr<AplBackstackExtensionObserverInterface> m_observer;
};

using AplBackstackExtensionPtr = std::shared_ptr<AplBackstackExtension>;

}  // namespace Backstack
}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_BACKSTACK_APLBACKSTACKEXTENSION_H
